#include "banks.h"
#define MYBANK BANK_0

#include "commands.h"
#include "b2_dsrutil.h"
#include <conio.h>
#include <string.h>

void handleDrives() {
  int i = 0;
  int cb = 0;
  
  while(dsrList[i].name[0] != 0) {
    cb = dsrList[i].crubase;
    cputs(uint2hex(cb));
    cputs(" -");
    while (cb == dsrList[i].crubase) {
      cputc(' ');
      cputs(dsrList[i].name);
      i++;
    }
    cputc('\n');
  }
}


