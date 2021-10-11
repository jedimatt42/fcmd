#ifndef _SYS_INFO_H
#define _SYS_INFO_H 1

#include "banking.h"

#include "b2_dsrutil.h"

/*
  System information structure
*/
struct __attribute__((__packed__)) SystemInformation {
  struct DeviceServiceRoutine* dsrList;
  struct DeviceServiceRoutine* currentDsr;
  const char* currentPath;
  unsigned int vdp_io_buf;
};

struct __attribute__((__packed__)) DisplayInformation {
  int isPal;
  int vdp_type;
  int displayWidth;
  int displayHeight;
  int imageAddr;
  int patternAddr;
  int colorAddr;
};

void sys_info(struct SystemInformation* info);
void display_info(struct DisplayInformation* info);

DECLARE_BANKED_VOID(sys_info, BANK(10), bk_sys_info, (struct SystemInformation* info), (info))
DECLARE_BANKED_VOID(display_info, BANK(10), bk_display_info, (struct DisplayInformation * info), (info))


#endif
