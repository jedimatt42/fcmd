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
  int open_error;
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
  r->open_error = 0;
  r->record_len = 0;
  r->record = 0;
  /* Fixed-record files are opened RELATIVE so the DSR honors the PAB
   * record number on every read.  This is true random access; no rewind or
   * sequential fallback is needed.  Try INT/FIX first, then DIS/FIX if the
   * DSR reports a data-type attribute mismatch. */
  static const int open_flags[2] = {
    DSR_TYPE_INPUT | DSR_TYPE_INTERNAL | DSR_TYPE_RELATIVE,
    DSR_TYPE_INPUT | DSR_TYPE_RELATIVE
  };
  unsigned int err = dsr_open(dsr, &r->pab, path, open_flags[0], RECORD_SIZE);
  if (err == 0x40) {
    err = dsr_open(dsr, &r->pab, path, open_flags[1], RECORD_SIZE);
  }
  if (err) {
    r->open_error = (int)err;
    r->error = (int)err;
    return -1;
  }
  r->record_len = (int)r->pab.RecordLength;
  if (r->record_len == 0) {
    /* Some DSRs do not report the record length on open.  Archives are
     * always 128-byte fixed files, so force it for the subsequent reads. */
    r->record_len = RECORD_SIZE;
    r->pab.RecordLength = RECORD_SIZE;
  }
  if (r->record_len != RECORD_SIZE) {
    r->open_error = 2; /* opened, but not a 128-byte fixed file */
    r->error = 2;
    dsr_close(dsr, &r->pab);
    return -1;
  }
  return 0;
}

static inline int ar_fill(struct ArcReader* r) {
  if (r->error || r->eof) {
    return -1;
  }
  /* Clear stale DSR error bits (the top three bits) but keep the file
   * attribute flags.  Some DSRs leave the EOF error from the size scan in
   * the PAB status, which otherwise makes every later successful read look
   * like an error. */
  r->pab.Status &= 0x1F;
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

/* Fixed-record files are opened RELATIVE, so the DSR uses the PAB record
 * number directly.  No rewind or forward scan is required. */
static inline int ar_seek(struct ArcReader* r, unsigned long offset) {
  r->error = 0;
  r->eof = 0;
  r->record = offset >> 7;
  r->pos = 0;
  r->len = 0;
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

/* Determine the archive payload size using only 128-byte record reads.
 * Archives are fixed 128-byte-record files.  This avoids Level 2 direct
 * input, which can report block counts that include the FIAD header. */
static inline int ar_size(struct ArcReader* r, unsigned long* size) {
  unsigned long rec = 0;
  r->error = 0;
  r->eof = 0;
  while (rec < 65536UL) {
    r->pab.Status &= 0x1F;
    unsigned int err = dsr_read_cpu(r->dsr, &r->pab, (int)rec, (char*)r->buf);
    if (err) {
      break;
    }
    if (r->pab.CharCount == 0) {
      break;
    }
    rec++;
  }
  r->pab.Status &= 0x1F;
  r->error = 0;
  r->eof = 0;
  r->record = 0;
  r->len = 0;
  r->pos = 0;
  *size = rec * (unsigned long)RECORD_SIZE;
  return 0;
}

#endif
