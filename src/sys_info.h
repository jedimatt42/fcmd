#ifndef _SYS_INFO_H
#define _SYS_INFO_H 1

#include "banking.h"

#include "dsrutil.h"

/*
  System information structure
*/
struct SystemInformation {
  struct DeviceServiceRoutine* dsrList;
  struct DeviceServiceRoutine* currentDsr;
  const char* currentPath;
  unsigned int vdp_io_buf;
};

void sys_info(struct SystemInformation* info);

DECLARE_BANKED_VOID(sys_info, BANK(10), bk_sys_info, (struct SystemInformation* info), (info))


#endif
