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
    tputs(uint2hex(cb));
    tputs(" -");
    while (cb == dsrList[i].crubase) {
      tputc(' ');
      tputs(dsrList[i].name);
      i++;
    }
    tputc('\n');
  }
}


