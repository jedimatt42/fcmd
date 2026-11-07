#include <fc_api.h>
#include "fcarc.h"
#include "fcarc_util.h"
#include "fcarc_path.h"
#include "fcarc_io.h"
#include "fcarc_lzw.h"
#include "fcarc_file.h"
#include "fcarc_sel.h"
#include "fcarc_msg.h"

struct FcInfo {
  unsigned long size;
  unsigned long dir_offset;
  unsigned long dir_size;
  unsigned int count;
  unsigned int dir_crc;
  unsigned int version;
};

static int out_sink(void* ctx, const unsigned char* data, int len) {
  struct OutFile* o = (struct OutFile*)ctx;
  int i;
  for (i = 0; i < len; i++) {
    if (o->remaining == 0) {
      o->error = 1;
      return 1;
    }
    o->crc = crc16_byte(o->crc, data[i]);
    o->remaining--;
  }
  if (out_write_block(o, data, len)) {
    o->error = 1;
    return 1;
  }
  return 0;
}

static int magic_eq(const unsigned char* a, const unsigned char* b, int n) {
  int i;
  for (i = 0; i < n; i++) {
    if (a[i] != b[i]) {
      return 0;
    }
  }
  return 1;
}

static int fc_read_trailer(struct ArcReader* r, struct DeviceServiceRoutine* dsr,
                           const char* path, struct FcInfo* info) {
  unsigned long size;
  if (ar_size(r, &size)) {
    term_puts(msg_error);
    term_puts(msg_file_size);
    return 1;
  }
  if (size < FCARC_TRAILER_SIZE) {
    term_puts(msg_error);
    term_puts(msg_truncated);
    return 1;
  }

  unsigned char t[FCARC_TRAILER_SIZE];
  unsigned long off = size - FCARC_TRAILER_SIZE;
  int found = 0;
  int tries;
  for (tries = 0; tries < 8 && off >= FCARC_TRAILER_SIZE; tries++) {
    r->error = 0;
    r->eof = 0;
    if (ar_seek(r, off) == 0
        && ar_read(r, t, FCARC_TRAILER_SIZE) == FCARC_TRAILER_SIZE
        && magic_eq(t, fcarc_trailer_magic, 8)) {
      found = 1;
      break;
    }
    off -= FCARC_TRAILER_SIZE;
  }
  if (!found) {
    term_puts(msg_error);
    term_puts(msg_not_fcarc);
    return 1;
  }

  unsigned long actual_size = off + FCARC_TRAILER_SIZE;
  info->version = le16(t + 8);
  info->dir_offset = le32(t + 12);
  info->dir_size = le32(t + 16);
  info->count = le16(t + 20);
  unsigned long psize = le32(t + 24);
  info->dir_crc = le16(t + 28);
  info->size = actual_size;
  if (info->version != FCARC_VERSION || psize != actual_size) {
    term_puts(msg_error);
    term_puts(msg_bad_format);
    return 1;
  }
  r->error = 0;
  r->eof = 0;
  unsigned char hv[12];
  if (ar_seek(r, 0) || ar_read(r, hv, 12) != 12
      || !magic_eq(hv, fcarc_magic, 8)
      || le16(hv + 8) != FCARC_VERSION) {
    term_puts(msg_error);
    term_puts(msg_bad_format);
    return 1;
  }
  if (info->count > FCARC_MAX_ENTRIES) {
    term_puts(msg_error);
    term_puts(msg_too_many);
    return 1;
  }
  if (info->dir_offset < FCARC_HEADER_SIZE
      || info->dir_offset + info->dir_size > actual_size - FCARC_TRAILER_SIZE) {
    term_puts(msg_error);
    term_puts(msg_bad_format);
    return 1;
  }
  return 0;
}

static int fc_read_entry(struct ArcReader* r, struct ArcEntry* e) {
  unsigned char h[FCARC_ENTRY_FIXED];
  if (ar_read(r, h, FCARC_ENTRY_FIXED) != FCARC_ENTRY_FIXED) {
    return 1;
  }
  int plen = h[0];
  if (plen < 1 || plen > MAX_PATH) {
    return 1;
  }
  e->kind = h[1];
  e->method = h[2];
  e->flags = h[3];
  e->recs = h[4];
  e->eof = h[5];
  e->reclen = h[6];
  e->sectors = ((unsigned int)h[7] << 8) | h[8];
  e->l3 = (unsigned int)h[9] | ((unsigned int)h[10] << 8);
  e->original_size = le32(h + 12);
  e->stored_size = le32(h + 16);
  e->data_offset = le32(h + 20);
  e->crc = le16(h + 24);
  if (ar_read(r, (unsigned char*)e->path, plen) != plen) {
    return 1;
  }
  e->path[plen] = 0;
  if (!path_validate(e->path)) {
    return 1;
  }
  if (e->kind != 0 && e->kind != 1) {
    return 1;
  }
  if (e->method != 0 && e->method != 1) {
    return 1;
  }
  return 0;
}

static int fc_extract_file(struct ArcReader* r, struct DeviceServiceRoutine* dsr,
                           unsigned int iocode, const char* parent,
                           const char* name, struct ArcEntry* e) {
  struct FileMeta m;
  m.flags = e->flags;
  m.recs = e->recs;
  m.eof = e->eof;
  m.reclen = e->reclen;
  m.l3 = e->l3;
  m.original_size = e->original_size;
  m.crc = e->crc;

  struct OutFile* of = &g_outfile;
  if (out_open(of, dsr, iocode, parent, name, &m)) {
    term_puts(msg_error);
    term_puts(msg_create);
    term_puts(name);
    term_putc('\n');
    return 1;
  }

  if (e->method == 1) {
    if (lzw_decode(r, out_sink, of) != 0) {
      term_puts(msg_error);
      term_puts(msg_crc);
      term_puts(e->path);
      term_putc('\n');
      return 1;
    }
  } else {
    unsigned long n = e->stored_size;
    unsigned char buf[128];
    while (n > 0) {
      int chunk = n > 128 ? 128 : (int)n;
      if (ar_read(r, buf, chunk) != chunk) {
        return 1;
      }
      if (out_sink(of, buf, chunk)) {
        return 1;
      }
      n -= chunk;
    }
  }

  if (of->remaining != 0 || of->error || of->crc != e->crc) {
    term_puts(msg_error);
    term_puts(msg_crc);
    term_puts(e->path);
    term_putc('\n');
    return 1;
  }
  if (out_close(of)) {
    term_puts(msg_error);
    term_puts(msg_write);
    term_puts(e->path);
    term_putc('\n');
    return 1;
  }
  return 0;
}

static int fc_extract(struct ArcReader* r, struct DeviceServiceRoutine* dsr,
                      const char* path, struct SelSet* sel,
                      struct DeviceServiceRoutine* odsr, unsigned int oiocode,
                      const char* curpath) {
  struct FcInfo info;
  if (fc_read_trailer(r, dsr, path, &info)) {
    return 1;
  }
  if (ar_seek(r, info.dir_offset)) {
    return 1;
  }
  unsigned int i;
  for (i = 0; i < info.count; i++) {
    struct ArcEntry e;
    if (fc_read_entry(r, &e)) {
      term_puts(msg_error);
      term_puts(msg_bad_format);
      return 1;
    }
    char* full = g_ws.scratch.full;
    if (path_join_rel(full, curpath, e.path, 128)) {
      term_puts(msg_error);
      term_puts(msg_path_too_long);
      term_puts(e.path);
      term_putc('\n');
      return 1;
    }
    if (e.kind == 1) {
      if (sel_dir_wanted(sel, e.path)) {
        if (ensure_dir_path(odsr, full)) {
          term_puts(msg_error);
          term_puts(msg_mkdir);
          term_puts(full);
          term_putc('\n');
          return 1;
        }
      }
      continue;
    }
    if (!sel_path_matches(sel, e.path)) {
      continue;
    }
    if (ensure_parent_dirs(odsr, full)) {
      term_puts(msg_error);
      term_puts(msg_mkdir);
      term_puts(full);
      term_putc('\n');
      return 1;
    }
    char* parent = g_ws.scratch.parent;
    char* name = g_ws.scratch.name;
    path_split(full, parent, name);
    unsigned long saved = ar_tell(r);
    if (e.data_offset + e.stored_size > info.dir_offset
        || ar_seek(r, e.data_offset)) {
      term_puts(msg_error);
      term_puts(msg_truncated);
      return 1;
    }
    if (fc_extract_file(r, odsr, oiocode, parent, name, &e)) {
      return 1;
    }
    if (ar_seek(r, saved)) {
      return 1;
    }
  }
  return 0;
}

int FCARC_SAMS(B_EXTRACT, fc_fcarc_extract(struct ArcReader* r, struct DeviceServiceRoutine* dsr,
                                const char* path, struct SelSet* sel,
                                struct DeviceServiceRoutine* odsr,
                                unsigned int oiocode, const char* curpath)) {
  return fc_extract(r, dsr, path, sel, odsr, oiocode, curpath);
}
