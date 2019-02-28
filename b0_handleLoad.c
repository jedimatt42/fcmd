#include "banks.h"

#define MYBANK BANK_0

#include "commands.h"
#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include <string.h>
#include <conio.h>

void handleLoad() {
  struct DeviceServiceRoutine* dsr = 0;

  char path[256];
  bk_parsePathParam(&dsr, path, PR_OPTIONAL);
  if (dsr == 0) {
    return;
  }

  if (!bk_existsFile(dsr, path)) {
    cputs("error, file not found: ");
    cputs(path);
    cputc('\n');
    return;
  }

  cputs("loading ");
  cputs(path);
  cputs("...\n");
} 