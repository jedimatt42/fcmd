#include "banks.h"
#define MYBANK BANK(4)

#include "b1_strutil.h"
#include "b0_parsing.h"
#include "b0_globals.h"
#include "commands.h"
#include "b8_terminal.h"
#include <string.h>

void handleCd() {
  struct DeviceServiceRoutine* dsr = 0;
  char path[256];
  bk_parsePathParam(0, &dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    tputs_rom("no path: drive or folder specified\n");
    return;
  }
  if (path[bk_strlen(path)-1] != '.') {
    bk_strcat(path, str2ram("."));
  }
  unsigned int stat = bk_existsDir(dsr, path);
  if (stat != 0) {
    tputs_rom("error, device/folder not found: ");
    bk_tputs_ram(path);
    bk_tputc('\n');
    return;
  }

  currentDsr = dsr;
  bk_strcpy(currentPath, path);
}

