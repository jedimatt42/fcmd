#include "banks.h"
#define MYBANK BANK(10)

#include "sys_info.h"
#include "globals.h"
#include <vdp.h>

void sys_info(struct SystemInformation* info) {
  info->dsrList = dsrList;
  info->currentDsr = currentDsr;
  info->currentPath = currentPath;
  info->vdp_io_buf = vdp_filesystem_file_buffer;
}
