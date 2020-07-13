#include "banks.h"
#define MYBANK BANK(4)

#include "b1cp_strutil.h"
#include "b0_parsing.h"
#include "b0_globals.h"
#include "commands.h"
#include "b1cp_terminal.h"
#include <string.h>

void handleCd() {
  struct DeviceServiceRoutine* dsr = 0;
  char path[256];
  bk_parsePathParam(&dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    tputs_rom("no path: drive or folder specified\n");
    return;
  }
  if (path[strlen(path)-1] != '.') {
    strcat(path, ".");
  }
  unsigned int stat = bk_existsDir(dsr, path);
  if (stat != 0) {
    tputs_rom("error, device/folder not found: ");
    tputs_ram(path);
    tputc('\n');
    return;
  }

  currentDsr = dsr;
  strcpy(currentPath, path);
}

