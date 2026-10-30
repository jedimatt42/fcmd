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

struct NullSink {
  unsigned long remaining;
  unsigned int crc;
  int error;
};

static int null_sink(void* ctx, const unsigned char* data, int len) {
  struct NullSink* n = (struct NullSink*)ctx;
  int i;
  for (i = 0; i < len; i++) {
    if (n->remaining == 0) {
      n->error = 1;
      return 1;
    }
    n->crc = crc16_byte(n->crc, data[i]);
    n->remaining--;
  }
  return 0;
}

static int fc_test(struct ArcReader* r, struct DeviceServiceRoutine* dsr,
                   const char* path) {
  struct FcInfo info;
  if (fc_read_trailer(r, dsr, path, &info)) {
    return 1;
  }

  if (ar_seek(r, info.dir_offset)) {
    return 1;
  }
  unsigned int crc = 0xFFFF;
  unsigned long left = info.dir_size;
  unsigned char buf[128];
  while (left > 0) {
    int n = left > 128 ? 128 : (int)left;
    if (ar_read(r, buf, n) != n) {
      term_puts(msg_error);
      term_puts(msg_truncated);
      return 1;
    }
    int i;
    for (i = 0; i < n; i++) {
      crc = crc16_byte(crc, buf[i]);
    }
    left -= n;
  }
  if (crc != info.dir_crc) {
    term_puts(msg_error);
    term_puts(msg_crc);
    term_puts(msg_directory);
    return 1;
  }

  if (ar_seek(r, info.dir_offset)) {
    return 1;
  }
  unsigned int bad = 0;
  unsigned int i;
  for (i = 0; i < info.count; i++) {
    struct ArcEntry e;
    if (fc_read_entry(r, &e)) {
      term_puts(msg_error_line);
      term_puts(msg_bad_format);
      return (int)bad + 1;
    }
    if (e.kind == 1) {
      continue;
    }
    unsigned long saved = ar_tell(r);
    int ok = 1;
    if (e.original_size != (unsigned long)e.sectors * SECTOR_SIZE) {
      ok = 0;
    }
    if (ok && (e.data_offset + e.stored_size > info.dir_offset
               || ar_seek(r, e.data_offset))) {
      ok = 0;
    }
    if (ok) {
      struct NullSink ns;
      ns.remaining = e.original_size;
      ns.crc = 0xFFFF;
      ns.error = 0;
      if (e.method == 1) {
        if (lzw_decode(r, null_sink, &ns) != 0) {
          ok = 0;
        }
      } else {
        unsigned long n = e.stored_size;
        while (n > 0) {
          int chunk = n > 128 ? 128 : (int)n;
          if (ar_read(r, buf, chunk) != chunk) {
            ok = 0;
            break;
          }
          if (null_sink(&ns, buf, chunk)) {
            ok = 0;
            break;
          }
          n -= chunk;
        }
      }
      if (ok && (ns.remaining != 0 || ns.error || ns.crc != e.crc)) {
        ok = 0;
      }
    }
    if (ok) {
      term_puts(msg_ok_line);
    } else {
      term_puts(msg_error_line);
      bad++;
    }
    term_puts(e.path);
    term_putc('\n');
    if (ar_seek(r, saved)) {
      term_puts(msg_error_line);
      term_puts(msg_bad_format);
      return (int)bad + 1;
    }
  }
  return (int)bad;
}

int FCARC_SAMS(B_TEST, fc_fcarc_test(struct ArcReader* r, struct DeviceServiceRoutine* dsr,
                             const char* path)) {
  return fc_test(r, dsr, path);
}
