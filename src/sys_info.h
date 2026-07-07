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

struct DisplayInformation {
  int isPal;
  int vdp_type;
  int displayWidth;
  int displayHeight;
  int imageAddr;
  int patternAddr;
  int colorAddr;
  int spritePatternAddr;
};

void sys_info(struct SystemInformation* info);
void sys_display_info(struct DisplayInformation* info);

DECLARE_BANKED_VOID(sys_info, BANK(10), bk_sys_info, (struct SystemInformation* info), (info))
DECLARE_BANKED_VOID(sys_display_info, BANK(10), bk_display_info, (struct DisplayInformation * info), (info))


#endif
