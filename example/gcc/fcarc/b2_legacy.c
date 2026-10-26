#include <fc_api.h>
#include "fcarc.h"
#include "fcarc_util.h"
#include "fcarc_path.h"
#include "fcarc_io.h"
#include "fcarc_lzw.h"
#include "fcarc_file.h"
#include "fcarc_sel.h"
#include "fcarc_msg.h"

struct LegacyDirOnly {
  int pos;
  int done;
  int error;
  unsigned char* dir;
};

static int legacy_dir_sink(void* ctx, const unsigned char* data, int len) {
  struct LegacyDirOnly* d = (struct LegacyDirOnly*)ctx;
  int i;
  for (i = 0; i < len; i++) {
    if (d->pos >= LEGACY_DIR_BYTES) {
      d->error = 1;
      return 1;
    }
    d->dir[d->pos++] = data[i];
    if ((d->pos & 0xFF) == 0) {
      int p = d->pos - 4;
      if (d->dir[p] == 'E' && d->dir[p + 1] == 'N'
          && d->dir[p + 2] == 'D' && d->dir[p + 3] == '!') {
        d->done = 1;
        return 1;
      }
    }
  }
  return 0;
}

static int legacy_read_dir_compressed(struct ArcReader* r, unsigned char* dir,
                                     int* dir_sectors) {
  struct LegacyDirOnly d;
  d.pos = 0;
  d.done = 0;
  d.error = 0;
  d.dir = dir;
  int rc = lzw_decode(r, legacy_dir_sink, &d);
  if (d.error || !d.done || (d.pos & 0xFF) != 0) {
    return 1;
  }
  if (rc != 1 && rc != 0) {
    return 1;
  }
  *dir_sectors = d.pos >> 8;
  return 0;
}

static int legacy_read_dir_plain(struct ArcReader* r, unsigned char* dir,
                                int* dir_sectors) {
  int s;
  for (s = 0; s < 10; s++) {
    if (ar_read(r, dir + s * SECTOR_SIZE, SECTOR_SIZE) != SECTOR_SIZE) {
      return 1;
    }
    int p = s * SECTOR_SIZE + 0xFC;
    if (dir[p] == 'E' && dir[p + 1] == 'N'
        && dir[p + 2] == 'D' && dir[p + 3] == '!') {
      *dir_sectors = s + 1;
      return 0;
    }
  }
  return 1;
}

static int legacy_count_entries(const unsigned char* dir, int dir_sectors) {
  int count = 0;
  int s;
  for (s = 0; s < dir_sectors; s++) {
    int off;
    for (off = 0; off < 0xFC; off += LEGACY_ENTRY_SIZE) {
      const unsigned char* p = dir + s * SECTOR_SIZE + off;
      if (p[0] == 0) {
        return count;
      }
      if (count >= LEGACY_MAX_ENTRIES) {
        return -1;
      }
      count++;
    }
  }
  return count;
}

static const unsigned char* legacy_entry_ptr(const unsigned char* dir, int index) {
  int s = 0;
  int off = index;
  while (off >= 14) {
    off -= 14;
    s++;
  }
  off = off * LEGACY_ENTRY_SIZE;
  return dir + s * SECTOR_SIZE + off;
}

static void legacy_entry_fields(const unsigned char* p, char* name,
                                struct FileMeta* m, unsigned int* sectors) {
  int i;
  for (i = 0; i < 10; i++) {
    name[i] = (char)p[i];
  }
  name[10] = 0;
  int last = 9;
  while (last >= 0 && name[last] == ' ') {
    name[last] = 0;
    last--;
  }
  m->flags = p[10];
  m->recs = p[11];
  *sectors = ((unsigned int)p[12] << 8) | p[13];
  m->eof = p[14];
  m->reclen = p[15];
  m->l3 = (unsigned int)p[16] | ((unsigned int)p[17] << 8);
  m->original_size = (unsigned long)(*sectors) * SECTOR_SIZE;
  m->crc = 0;
}

static int legacy_name_valid(const char* name) {
  int len = str_len(name);
  if (len < 1 || len > 10) {
    return 0;
  }
  int i;
  for (i = 0; i < len; i++) {
    char c = name[i];
    if (c == ' ' || c == '.' || c == '/' || c == '\\' || c < 32) {
      return 0;
    }
  }
  return 1;
}

static int legacy_list(struct ArcReader* r) {
  unsigned char dirbuf[LEGACY_DIR_BYTES];
  int dir_sectors;
  if (ar_seek(r, 0)) {
    return 1;
  }
  int c = ar_getc(r);
  if (c < 0) {
    return 1;
  }
  if (c == 0x80) {
    if (ar_seek(r, 0) || legacy_read_dir_compressed(r, dirbuf, &dir_sectors)) {
      term_puts(msg_error);
      term_puts(msg_bad_format);
      return 1;
    }
  } else {
    if (ar_seek(r, 0) || legacy_read_dir_plain(r, dirbuf, &dir_sectors)) {
      term_puts(msg_error);
      term_puts(msg_bad_format);
      return 1;
    }
  }
  int count = legacy_count_entries(dirbuf, dir_sectors);
  if (count < 0) {
    term_puts(msg_error);
    term_puts(msg_too_many);
    return 1;
  }
  term_puts(msg_ti_archiver);
  term_puts(str_from_uint((unsigned int)count));
  term_puts(msg_entries);
  term_puts(msg_newline);
  int i;
  for (i = 0; i < count; i++) {
    const unsigned char* p = legacy_entry_ptr(dirbuf, i);
    char name[11];
    struct FileMeta m;
    unsigned int sectors;
    legacy_entry_fields(p, name, &m, &sectors);
    term_puts((m.flags & 0x01) ? msg_program : (m.flags & 0x80) ? msg_var : msg_fix);
    term_puts(str_from_uint(sectors));
    term_puts(msg_sectors);
    term_puts(name);
    term_putc('\n');
  }
  return 0;
}

struct LegacySink {
  int mode;
  int dir_pos;
  int dir_sectors;
  int count;
  int index;
  unsigned long remaining;
  int writing;
  int extract;
  int error;
  struct SelSet* sel;
  struct DeviceServiceRoutine* dsr;
  unsigned int iocode;
  const char* curpath;
  unsigned char* dir;
  struct OutFile out;
};

static int legacy_start_next(struct LegacySink* s) {
  while (s->index < s->count) {
    const unsigned char* p = legacy_entry_ptr(s->dir, s->index);
    char name[11];
    struct FileMeta m;
    unsigned int sectors;
    legacy_entry_fields(p, name, &m, &sectors);
    if (!legacy_name_valid(name)) {
      s->error = 1;
      return 1;
    }
    s->remaining = (unsigned long)sectors * SECTOR_SIZE;
    s->writing = 0;
    int want = sel_path_matches(s->sel, name);
    if (s->extract && want) {
      if (out_open(&s->out, s->dsr, s->iocode, s->curpath, name, &m)) {
        s->error = 1;
        return 1;
      }
      s->writing = 1;
      if (s->remaining == 0) {
        if (out_close(&s->out)) {
          s->error = 1;
          return 1;
        }
        s->writing = 0;
      }
    }
    if (s->remaining == 0) {
      s->index++;
      continue;
    }
    return 0;
  }
  s->mode = 2;
  return 0;
}

static int legacy_sink(void* ctx, const unsigned char* data, int len) {
  struct LegacySink* s = (struct LegacySink*)ctx;
  while (len > 0) {
    if (s->mode == 0) {
      int space = SECTOR_SIZE - (s->dir_pos & 0xFF);
      int n = len < space ? len : space;
      if (s->dir_pos + n > LEGACY_DIR_BYTES) {
        s->error = 1;
        return 1;
      }
      memcpy(s->dir + s->dir_pos, (void*)data, n);
      s->dir_pos += n;
      data += n;
      len -= n;
      if ((s->dir_pos & 0xFF) == 0) {
        int p = s->dir_pos - 4;
        if (s->dir[p] == 'E' && s->dir[p + 1] == 'N'
            && s->dir[p + 2] == 'D' && s->dir[p + 3] == '!') {
          s->dir_sectors = s->dir_pos >> 8;
          s->count = legacy_count_entries(s->dir, s->dir_sectors);
          if (s->count < 0) {
            s->error = 1;
            return 1;
          }
          s->index = 0;
          s->mode = 1;
          if (legacy_start_next(s)) {
            return 1;
          }
        }
      }
      continue;
    }
    if (s->mode == 1) {
      if (s->remaining == 0) {
        s->index++;
        if (legacy_start_next(s)) {
          return 1;
        }
        if (s->mode == 2) {
          return 0;
        }
        continue;
      }
      int n = len;
      if ((unsigned long)n > s->remaining) {
        n = (int)s->remaining;
      }
      if (s->writing) {
        if (out_write_block(&s->out, data, n)) {
          s->error = 1;
          return 1;
        }
      }
      data += n;
      len -= n;
      s->remaining -= (unsigned long)n;
      if (s->remaining == 0) {
        if (s->writing) {
          if (out_close(&s->out)) {
            s->error = 1;
            return 1;
          }
          s->writing = 0;
        }
      }
      continue;
    }
    return 0;
  }
  return 0;
}

static int legacy_extract_compressed(struct ArcReader* r, struct SelSet* sel,
                                     struct DeviceServiceRoutine* dsr,
                                     unsigned int iocode, const char* curpath,
                                     int extract) {
  unsigned char dirbuf[LEGACY_DIR_BYTES];
  struct LegacySink s;
  s.mode = 0;
  s.dir_pos = 0;
  s.dir_sectors = 0;
  s.count = 0;
  s.index = 0;
  s.remaining = 0;
  s.writing = 0;
  s.extract = extract;
  s.error = 0;
  s.sel = sel;
  s.dsr = dsr;
  s.iocode = iocode;
  s.curpath = curpath;
  s.dir = dirbuf;
  s.out.open = 0;

  int rc = lzw_decode(r, legacy_sink, &s);
  if (s.error || rc < 0) {
    term_puts(msg_error);
    term_puts(msg_bad_format);
    return 1;
  }
  if (s.mode == 0) {
    term_puts(msg_error);
    term_puts(msg_truncated);
    return 1;
  }
  if (s.index < s.count && s.remaining != 0) {
    term_puts(msg_error);
    term_puts(msg_truncated);
    return 1;
  }
  return 0;
}

static int legacy_extract_plain(struct ArcReader* r, struct SelSet* sel,
                                struct DeviceServiceRoutine* dsr,
                                unsigned int iocode, const char* curpath,
                                int extract) {
  unsigned char dirbuf[LEGACY_DIR_BYTES];
  int dir_sectors;
  if (ar_seek(r, 0) || legacy_read_dir_plain(r, dirbuf, &dir_sectors)) {
    term_puts(msg_error);
    term_puts(msg_bad_format);
    return 1;
  }
  int count = legacy_count_entries(dirbuf, dir_sectors);
  if (count < 0) {
    term_puts(msg_error);
    term_puts(msg_too_many);
    return 1;
  }
  unsigned long sector = (unsigned long)dir_sectors;
  int i;
  for (i = 0; i < count; i++) {
    const unsigned char* p = legacy_entry_ptr(dirbuf, i);
    char name[11];
    struct FileMeta m;
    unsigned int sectors;
    legacy_entry_fields(p, name, &m, &sectors);
    if (!legacy_name_valid(name)) {
      term_puts(msg_error);
      term_puts(msg_name_invalid);
      return 1;
    }
    unsigned long off = sector * SECTOR_SIZE;
    sector += sectors;
    if (!extract || !sel_path_matches(sel, name)) {
      continue;
    }
    if (ar_seek(r, off)) {
      return 1;
    }
    struct OutFile of;
    if (out_open(&of, dsr, iocode, curpath, name, &m)) {
      term_puts(msg_error);
      term_puts(msg_create);
      term_puts(name);
      term_putc('\n');
      return 1;
    }
    unsigned long left = m.original_size;
    while (left > 0) {
      unsigned char buf[SECTOR_SIZE];
      if (ar_read(r, buf, SECTOR_SIZE) != SECTOR_SIZE) {
        term_puts(msg_error);
        term_puts(msg_truncated);
        return 1;
      }
      if (out_write_block(&of, buf, SECTOR_SIZE)) {
        return 1;
      }
      left -= SECTOR_SIZE;
    }
    if (out_close(&of)) {
      return 1;
    }
    if (!g_quiet) {
      term_puts(msg_extracting);
      term_puts(name);
      term_putc('\n');
    }
  }
  return 0;
}

int FC_SAMS(2, fc_legacy_list(struct ArcReader* r)) {
  return legacy_list(r);
}

int FC_SAMS(2, fc_legacy_extract(struct ArcReader* r, struct SelSet* sel,
                                struct DeviceServiceRoutine* dsr,
                                unsigned int iocode, const char* curpath,
                                int extract)) {
  if (ar_seek(r, 0)) {
    return 1;
  }
  int c = ar_getc(r);
  if (c < 0) {
    return 1;
  }
  if (c == 0x80) {
    if (ar_seek(r, 0)) {
      return 1;
    }
    return legacy_extract_compressed(r, sel, dsr, iocode, curpath, extract);
  }
  return legacy_extract_plain(r, sel, dsr, iocode, curpath, extract);
}

int FC_SAMS(2, fc_legacy_test(struct ArcReader* r, struct DeviceServiceRoutine* dsr,
                              const char* path)) {
  if (ar_seek(r, 0)) {
    return 1;
  }
  int c = ar_getc(r);
  if (c < 0) {
    return 1;
  }
  struct SelSet sel;
  sel.count = 0;
  if (c == 0x80) {
    if (ar_seek(r, 0)) {
      return 1;
    }
    return legacy_extract_compressed(r, &sel, dsr, 0, "", 0);
  }
  if (ar_seek(r, 0)) {
    return 1;
  }
  unsigned char dirbuf[LEGACY_DIR_BYTES];
  int dir_sectors;
  if (legacy_read_dir_plain(r, dirbuf, &dir_sectors)) {
    return 1;
  }
  int count = legacy_count_entries(dirbuf, dir_sectors);
  if (count < 0) {
    return 1;
  }
  unsigned long total = (unsigned long)dir_sectors;
  int i;
  for (i = 0; i < count; i++) {
    const unsigned char* p = legacy_entry_ptr(dirbuf, i);
    char name[11];
    struct FileMeta m;
    unsigned int sectors;
    legacy_entry_fields(p, name, &m, &sectors);
    if (!legacy_name_valid(name)) {
      return 1;
    }
    total += sectors;
  }
  unsigned long size;
  if (fc_file_size(dsr, path, &size)) {
    return 1;
  }
  if (total * SECTOR_SIZE > size) {
    return 1;
  }
  return 0;
}
