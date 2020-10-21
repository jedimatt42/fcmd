#include "banks.h"
#define MYBANK BANK(10)

#include "b10_sys_info.h"
#include "b0_globals.h"

void sys_info(struct SystemInformation* info) {
    info->currentDsr = currentDsr;
    info->currentPath = currentPath;
    info->displayWidth = displayWidth;
    info->displayHeight = ( displayWidth == 80 ? 30 : 24 );
    info->isPal = pal;
}
