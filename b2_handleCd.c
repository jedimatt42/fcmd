#include "banks.h"
#define MYBANK BANK_2

#include "b1cp_strutil.h"
#include "b0_parsing.h"
#include "b0_globals.h"
#include "b2_commands.h"
#include <conio.h>
#include <string.h>

void handleCd() {
  struct DeviceServiceRoutine* dsr = 0;
  char path[256];
  bk_parsePathParam(&dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    cputs("no path: drive or folder specified\n");
    return;
  }
  if (path[strlen(path)-1] != '.') {
    strcat(path, ".");
  }
  unsigned char stat = existsDir(dsr, path);
  if (stat != 0) {
    cputs("error, device/folder not found: ");
    cputs(path);
    cputc('\n');
    return;
  }
  
  currentDsr = dsr;
  strcpy(currentPath, path);
}

