#include "banks.h"

#define MYBANK BANK(0)

#include "commands.h"
#include "b0_main.h"
#include "b0_globals.h"
#include "b10_parsing.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b8_setupScreen.h"
#include <string.h>
#include <vdp.h>
#include <conio.h>

void handleLoad() {
  struct DeviceServiceRoutine* dsr = 0;

  char path[256];
  bk_parsePathParam(0, &dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    tputs_rom("no file specified\n");
    return;
  }

  // don't require exists if a PI.HTTP URL...
  if (path[0] != 'P' || path[1] != 'I') {
    if (!bk_existsFile(dsr, path)) {
      tputs_rom("error, file not found: ");
      bk_tputs_ram(path);
      bk_tputc('\n');
      return;
    }
  }

  bk_setupScreen(0);

  // TODO load ea-utils into >2000

  bk_dsr_ea5load(dsr, path);
}