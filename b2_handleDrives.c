#include "banks.h"
#define MYBANK BANK_2

#include "b2_dsrutil.h"
#include "b1_libti99.h"

void handleDrives() {
  int i = 0;
  int cb = 0;
  
  while(dsrList[i].name[0] != 0) {
    cb = dsrList[i].crubase;
    bk_cputs(bk_uint2hex(cb));
    CCPUTS(" -");
    while (cb == dsrList[i].crubase) {
      bk_cputc(' ');
      bk_cputs(dsrList[i].name);
      i++;
    }
    bk_cputc('\n');
  }
}


