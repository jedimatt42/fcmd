#include "banks.h"

#define MYBANK BANK_0

#include "commands.h"
#include "b0_main.h"
#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include <string.h>
#include <vdp.h>
#include <conio.h>

void handleLoad() {
  struct DeviceServiceRoutine* dsr = 0;

  char path[256];
  bk_parsePathParam(&dsr, path, PR_OPTIONAL);
  if (dsr == 0) {
    return;
  }

  // TODO, don't require exists if a PI.HTTP URL... 
  if (path[0] != 'P' || path[1] != 'I') {
    if (!bk_existsFile(dsr, path)) {
      tputs("error, file not found: ");
      tputs(path);
      tputc('\n');
      return;
    }
  }

  // TODO - test that it is ea5-ish

  resetF18A();
  set_graphics(0);

  // erase first 4k of vdp
  vdpmemset(0,0,4192);

  // ea clears screen with 0x20 (space characters)  
  clrscr();
  // load character set
  charset();
  // TODO load ea copyright and cursor

  // set colors
  vdpmemset(0x0380, 0x13, 32);

  // TODO load ea-utils into >2000

  bk_dsr_ea5load(dsr, path);
} 