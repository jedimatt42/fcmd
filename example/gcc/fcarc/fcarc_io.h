#ifndef _FCARC_IO_H
#define _FCARC_IO_H 1

#include <fc_api.h>
#include "fcarc.h"
#include "fcarc_path.h"

#ifndef ADDINFO
#define ADDINFO ((struct AddInfo*)0x8320)
#endif

#ifdef HOST_TEST
extern unsigned char* host_addinfo_buffer;
#define SET_ADDINFO_BUFFER(p) (host_addinfo_buffer = (unsigned char*)(p))
#else
#define SET_ADDINFO_BUFFER(p) (ADDINFO->buffer = (unsigned int)(p))
#endif

struct ArcWriter {
  struct PAB pab;
  struct DeviceServiceRoutine* dsr;
  unsigned char buf[RECORD_SIZE];
  int pos;
  int error;
  unsigned long total;
};

struct ArcReader {
  struct PAB pab;
  struct DeviceServiceRoutine* dsr;
  unsigned char buf[RECORD_SIZE];
  int pos;
  int len;
  int record_len;
  int error;
  int eof;
  unsigned long record;
};

static inline int aw_open(struct ArcWriter* w, struct DeviceServiceRoutine* dsr,
                          const char* path, int flags) {
  w->dsr = dsr;
  w->pos = 0;
  w->error = 0;
  w->total = 0;
  unsigned int err = dsr_open(dsr, &w->pab, path, flags, RECORD_SIZE);
  if (err) {
    w->error = (int)err;
    return -1;
  }
  return 0;
}

static inline int aw_flush(struct ArcWriter* w) {
  if (w->error) {
    return -1;
  }
  if (w->pos == 0) {
    return 0;
  }
  while (w->pos < RECORD_SIZE) {
    w->buf[w->pos++] = 0;
  }
  unsigned int err = dsr_write(w->dsr, &w->pab, (char*)w->buf, RECORD_SIZE);
  w->pos = 0;
  if (err) {
    w->error = (int)err;
    return -1;
  }
  return 0;
}

static inline int aw_putc(struct ArcWriter* w, int c) {
  if (w->error) {
    return -1;
  }
  if (w->pos >= RECORD_SIZE) {
    if (aw_flush(w)) {
      return -1;
    }
  }
  w->buf[w->pos++] = (unsigned char)c;
  w->total++;
  return 0;
}

static inline int aw_write(struct ArcWriter* w, const unsigned char* data, int len) {
  int i;
  for (i = 0; i < len; i++) {
    if (aw_putc(w, data[i])) {
      return -1;
    }
  }
  return 0;
}

static inline int aw_pad128(struct ArcWriter* w) {
  while ((w->total & 0x7F) != 0) {
    if (aw_putc(w, 0)) {
      return -1;
    }
  }
  return aw_flush(w);
}

static inline int aw_close(struct ArcWriter* w) {
  int rc = aw_flush(w);
  unsigned int err = dsr_close(w->dsr, &w->pab);
  if (err && !w->error) {
    w->error = (int)err;
    rc = -1;
  }
  return rc;
}

static inline int ar_open(struct ArcReader* r, struct DeviceServiceRoutine* dsr,
                          const char* path) {
  r->dsr = dsr;
  r->pos = 0;
  r->len = 0;
  r->error = 0;
  r->eof = 0;
  r->record = 0;
  unsigned int err = dsr_open(dsr, &r->pab, path, DSR_TYPE_INPUT, 0);
  if (err) {
    r->error = (int)err;
    return -1;
  }
  r->record_len = (int)r->pab.RecordLength;
  if (r->record_len != RECORD_SIZE) {
    r->error = 1;
    dsr_close(dsr, &r->pab);
    return -1;
  }
  return 0;
}

static inline int ar_fill(struct ArcReader* r) {
  if (r->error || r->eof) {
    return -1;
  }
  unsigned int err = dsr_read_cpu(r->dsr, &r->pab, (int)r->record, (char*)r->buf);
  if (err) {
    r->error = (int)err;
    return -1;
  }
  r->len = (int)r->pab.CharCount;
  if (r->len <= 0) {
    r->eof = 1;
    return -1;
  }
  if (r->len > RECORD_SIZE) {
    r->len = RECORD_SIZE;
  }
  r->pos = 0;
  r->record++;
  return 0;
}

static inline int ar_getc(struct ArcReader* r) {
  if (r->pos >= r->len) {
    if (ar_fill(r)) {
      return -1;
    }
  }
  return (int)r->buf[r->pos++];
}

static inline int ar_read(struct ArcReader* r, unsigned char* dst, int len) {
  int i;
  for (i = 0; i < len; i++) {
    int c = ar_getc(r);
    if (c < 0) {
      return i;
    }
    dst[i] = (unsigned char)c;
  }
  return len;
}

static inline unsigned long ar_tell(struct ArcReader* r) {
  if (r->len > 0) {
    return ((r->record - 1) * (unsigned long)r->record_len) + (unsigned long)r->pos;
  }
  return r->record * (unsigned long)r->record_len;
}

static inline int ar_seek(struct ArcReader* r, unsigned long offset) {
  if (r->error) {
    return -1;
  }
  r->record = offset >> 7;
  r->pos = 0;
  r->len = 0;
  r->eof = 0;
  int skip = (int)(offset & 0x7F);
  if (skip) {
    if (ar_fill(r)) {
      return -1;
    }
    r->pos = skip;
  }
  return 0;
}

static inline int ar_close(struct ArcReader* r) {
  unsigned int err = dsr_close(r->dsr, &r->pab);
  if (err) {
    r->error = (int)err;
    return -1;
  }
  return 0;
}

/* Determine the size of a TI file with Level 2 direct input metadata. */
static inline int fc_file_size(struct DeviceServiceRoutine* dsr, const char* path,
                               unsigned long* size) {
  char parent[256];
  char name[16];
  path_split(path, parent, name);
  unsigned int iocode = path_to_iocode(path);
  unsigned int err = lvl2_setdir(dsr->crubase, iocode, parent);
  if (err) {
    return -1;
  }
  char bare[16];
  str_ncopy(bare, name, 15);
  bare[15] = 0;
  ADDINFO->first_sector = 0;
  ADDINFO->buffer = 0;
  ADDINFO->flags = 0;
  ADDINFO->recs_per_sec = 0;
  ADDINFO->eof_offset = 0;
  ADDINFO->rec_length = 0;
  ADDINFO->records = 0;
  err = lvl2_input(dsr->crubase, iocode, bare, 0, ADDINFO);
  if (err) {
    return -1;
  }
  *size = (unsigned long)ADDINFO->first_sector * SECTOR_SIZE;
  return 0;
}

#endif
