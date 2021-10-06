#ifndef _SINGLE_FILE_H
#define _SINGLE_FILE_H 1

#include "banking.h"

#include "b2_dsrutil.h"

void copySingleFile(struct DeviceServiceRoutine* srcdsr, char* srcpath, char* filename, struct DeviceServiceRoutine* dstdsr, char* dstpath);

DECLARE_BANKED_VOID(copySingleFile, BANK(5), bk_copySingleFile, (struct DeviceServiceRoutine* srcdsr, char* srcpath, char* filename, struct DeviceServiceRoutine* dstdsr, char* dstpath), (srcdsr, srcpath, filename, dstdsr, dstpath))


#endif
