#ifndef _SINGLE_FILE_H
#define _SINGLE_FILE_H 1

#include "banking.h"

#include "b2_dsrutil.h"

struct __attribute__((__packed__)) CopySpec {
  struct DeviceServiceRoutine* dsr;
  char* path;
  char* filename;
};

void copySingleFile(struct CopySpec* src, struct CopySpec* dst);

DECLARE_BANKED_VOID(copySingleFile, BANK(5), bk_copySingleFile, (struct CopySpec* src, struct CopySpec* dst), (src, dst))


#endif
