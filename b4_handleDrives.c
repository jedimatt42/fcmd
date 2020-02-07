#include "banks.h"
#define MYBANK BANK_4

#include "commands.h"
#include "b2_dsrutil.h"
#include "b1cp_terminal.h"
#include <string.h>

void handleDrives() {
  int i = 0;
  int cb = 0;
  
  while(dsrList[i].name[0] != 0) {
    cb = dsrList[i].crubase;
    tputs_ram(uint2hex(cb));
    tputs_rom(" -");
    while (cb == dsrList[i].crubase) {
      tputc(' ');
      tputs_ram(dsrList[i].name);
      i++;
    }
    tputc('\n');
  }
}


