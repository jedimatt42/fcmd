#include <fc_api.h>
#include "fcarc.h"
#include "fcarc_file.h"
#include "fcarc_util.h"
#include "fcarc_path.h"
#include "fcarc_io.h"

static void set_addinfo_meta(const struct FileMeta* m) {
  ADDINFO->first_sector = 0;
  ADDINFO->buffer = 0;
  ADDINFO->flags = m->flags;
  ADDINFO->recs_per_sec = m->recs;
  ADDINFO->eof_offset = m->eof;
  ADDINFO->rec_length = m->reclen;
  unsigned char* p = (unsigned char*)&ADDINFO->records;
  p[0] = (unsigned char)(m->l3 & 0xFF);
  p[1] = (unsigned char)((m->l3 >> 8) & 0xFF);
}

static int out_flush(struct OutFile* o) {
  if (o->error) {
    return 1;
  }
  if (o->pos > 0) {
    while (o->pos < SECTOR_SIZE) {
      o->buf[o->pos++] = 0;
    }
    ADDINFO->first_sector = (unsigned int)o->block;
    SET_ADDINFO_BUFFER(o->buf);
    unsigned int err = lvl2_output_cpu(o->dsr->crubase, o->iocode, o->name, 1, ADDINFO);
    if (err) {
      o->error = (int)err;
      return 1;
    }
    o->block++;
    o->pos = 0;
  }
  return 0;
}

int FC_SAMS(0, out_open(struct OutFile* o, struct DeviceServiceRoutine* dsr,
                        unsigned int iocode, const char* parent,
                        const char* name, const struct FileMeta* m)) {
  o->dsr = dsr;
  o->iocode = iocode;
  str_ncopy(o->name, (char*)name, 10);
  o->name[10] = 0;
  o->pos = 0;
  o->block = 0;
  o->open = 1;
  o->error = 0;
  o->remaining = m->original_size;
  o->crc = 0xFFFF;

  unsigned int err = lvl2_setdir(dsr->crubase, iocode, (char*)parent);
  if (err) {
    o->error = (int)err;
    return 1;
  }
  set_addinfo_meta(m);
  SET_ADDINFO_BUFFER(o->buf);
  err = lvl2_output_cpu(dsr->crubase, iocode, o->name, 0, ADDINFO);
  if (err) {
    o->error = (int)err;
    return 1;
  }
  return 0;
}

static int out_putc(struct OutFile* o, int c) {
  if (o->pos >= SECTOR_SIZE) {
    if (out_flush(o)) {
      return 1;
    }
  }
  o->buf[o->pos++] = (unsigned char)c;
  return 0;
}

int FC_SAMS(0, out_write_block(struct OutFile* o,
                               const unsigned char* data, int len)) {
  int i;
  for (i = 0; i < len; i++) {
    if (out_putc(o, data[i])) {
      return 1;
    }
  }
  return 0;
}

int FC_SAMS(0, out_close(struct OutFile* o)) {
  int rc = out_flush(o);
  o->open = 0;
  return rc;
}

static void pab_zero(struct PAB* pab) {
  str_set((char*)pab, 0, (int)sizeof(struct PAB));
}

int FC_SAMS(0, dir_exists(struct DeviceServiceRoutine* dsr, const char* path)) {
  struct PAB pab;
  pab_zero(&pab);
  unsigned int err = dsr_open(dsr, &pab, path,
                              DSR_TYPE_INPUT | DSR_TYPE_INTERNAL | DSR_TYPE_RELATIVE,
                              0);
  if (err) {
    return 0;
  }
  err = dsr_read(dsr, &pab, 0);
  dsr_close(dsr, &pab);
  return err == 0;
}

int FC_SAMS(0, ensure_dir_path(struct DeviceServiceRoutine* dsr, const char* path)) {
  char work[256];
  str_ncopy(work, (char*)path, 255);
  work[255] = 0;
  int len = str_len(work);
  while (len > 0 && work[len - 1] == '.') {
    len--;
    work[len] = 0;
  }
  int first = 0;
  while (first < len && work[first] != '.') {
    first++;
  }
  if (first >= len) {
    return 0;
  }
  int start = first + 1;
  while (start < len) {
    int end = start;
    while (end < len && work[end] != '.') {
      end++;
    }
    if (end > start) {
      char dir[256];
      int i;
      for (i = 0; i < end; i++) {
        dir[i] = work[i];
      }
      dir[end] = 0;
      if (!dir_exists(dsr, dir)) {
        char parent[256];
        for (i = 0; i < start; i++) {
          parent[i] = work[i];
        }
        parent[start] = 0;
        char name[16];
        int n = end - start;
        if (n > 10) {
          return 1;
        }
        for (i = 0; i < n; i++) {
          name[i] = work[start + i];
        }
        name[n] = 0;
        if (str_len(parent) > 39) {
          return 1;
        }
        unsigned int iocode = path_to_iocode(dir);
        unsigned int err = lvl2_setdir(dsr->crubase, iocode, parent);
        if (!err) {
          err = lvl2_mkdir(dsr->crubase, iocode, name);
        }
        if (err) {
          return 1;
        }
      }
    }
    start = end + 1;
  }
  return 0;
}

int FC_SAMS(0, ensure_parent_dirs(struct DeviceServiceRoutine* dsr,
                                  const char* fullpath)) {
  char parent[256];
  char name[16];
  path_split(fullpath, parent, name);
  return ensure_dir_path(dsr, parent);
}
