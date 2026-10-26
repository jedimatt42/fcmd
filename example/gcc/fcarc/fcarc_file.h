#ifndef _FCARC_FILE_H
#define _FCARC_FILE_H 1

#include <fc_api.h>
#include "fcarc.h"

struct FileMeta {
  unsigned char flags;
  unsigned char recs;
  unsigned char eof;
  unsigned char reclen;
  unsigned int l3;
  unsigned long original_size;
  unsigned int crc;
};

struct OutFile {
  struct DeviceServiceRoutine* dsr;
  unsigned int iocode;
  char name[16];
  unsigned char buf[SECTOR_SIZE];
  int pos;
  int block;
  int open;
  int error;
  unsigned long remaining;
  unsigned int crc;
};

/* Bank 0 provides these so every reader/create bank can share one copy. */
FC_SAMS_BANKED(0, int, out_open,
  (struct OutFile* o, struct DeviceServiceRoutine* dsr, unsigned int iocode,
   const char* parent, const char* name, const struct FileMeta* m),
  (o, dsr, iocode, parent, name, m))

FC_SAMS_BANKED(0, int, out_write_block,
  (struct OutFile* o, const unsigned char* data, int len),
  (o, data, len))

FC_SAMS_BANKED(0, int, out_close, (struct OutFile* o), (o))

FC_SAMS_BANKED(0, int, dir_exists,
  (struct DeviceServiceRoutine* dsr, const char* path),
  (dsr, path))

FC_SAMS_BANKED(0, int, ensure_dir_path,
  (struct DeviceServiceRoutine* dsr, const char* path),
  (dsr, path))

FC_SAMS_BANKED(0, int, ensure_parent_dirs,
  (struct DeviceServiceRoutine* dsr, const char* fullpath),
  (dsr, fullpath))

#endif
