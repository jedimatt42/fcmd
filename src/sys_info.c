#include "banks.h"
#define MYBANK BANK(10)

#include "sys_info.h"
#include "globals.h"
#include "detect_vdp.h"
#include "lvl2.h"
#include <vdp.h>

void sys_info(struct SystemInformation* info) {
  info->dsrList = dsrList;
  info->currentDsr = currentDsr;
  info->currentPath = currentPath;
  info->vdp_io_buf = VDPFBUF;
}

void sys_display_info(struct DisplayInformation* info) {
  info->displayWidth = displayWidth;
  info->displayHeight = displayHeight;
  info->isPal = pal;
  info->vdp_type = vdp_type;
  info->patternAddr = gPattern;
  info->imageAddr = gImage;
  info->colorAddr = gColor;
  info->spritePatternAddr = gSpritePat;
}
