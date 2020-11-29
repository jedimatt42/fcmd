#include "banks.h"
#define MYBANK BANK(4)

#include "commands.h"
#include "b2_dsrutil.h"
#include "b8_terminal.h"
#include "b0_globals.h"
#include <string.h>

void handleDrives() {
  int i = 0;
  int cb = 0;

  while(dsrList[i].name[0] != 0 && request_break == 0) {
    cb = dsrList[i].crubase;
    bk_tputs_ram(bk_uint2hex(cb));
    tputs_rom(" -");
    while (cb == dsrList[i].crubase) {
      bk_tputc(' ');
      bk_tputs_ram(dsrList[i].name);
      i++;
    }
    bk_tputc('\n');
  }
}


