#include "banks.h"

#define MYBANK BANK(0)

#include "commands.h"
#include "main.h"
#include "globals.h"
#include "parsing.h"
#include "strutil.h"
#include "terminal.h"
#include "dsrutil.h"
#include "lvl2.h"
#include "setupScreen.h"
#include <string.h>
#include <vdp.h>
#include <conio.h>

int handleLoad() {
  struct DeviceServiceRoutine* dsr = 0;

  char path[256];
  bk_parsePathParam(0, &dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    tputs_rom("no file specified\n");
  return 0;
  }

  // don't require exists if a PI.HTTP URL...
  if (path[0] != 'P' || path[1] != 'I') {
    if (!bk_existsFile(dsr, path)) {
      tputs_rom("error, file not found: ");
      bk_tputs_ram(path);
      bk_tputc('\n');
  return 0;
    }
  }

  bk_setupScreen(0);

  // TODO load ea-utils into >2000

  bk_dsr_ea5load(dsr, path);
  return 0;
}