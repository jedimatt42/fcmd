#ifndef _SYS_INFO_H
#define _SYS_INFO_H 1

#include "banking.h"

/*
  System information structure
*/
struct __attribute__((__packed__)) SystemInformation {
  int isPal;
  int displayWidth;
  int displayHeight;
  struct DeviceServiceRoutine* currentDsr;
  const char* currentPath;
};

void sys_info(struct SystemInformation* info);

DECLARE_BANKED_VOID(sys_info, BANK(10), bk_sys_info, (struct SystemInformation* info), (info))

#endif