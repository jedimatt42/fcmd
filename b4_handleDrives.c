#include "banks.h"
#define MYBANK BANK(4)

#include "commands.h"
#include "b2_dsrutil.h"
#include "b8_terminal.h"
#include "b0_globals.h"
#include <string.h>
#include "b1_strutil.h"

void handleDrives() {
  int addresses = 0;
  char* peek = bk_strtokpeek(0, ' ');
  if (0 == bk_strcmpi(str2ram("/a"), peek)) {
    addresses = 1;
  }

  int i = 0;
  int cb = 0;

  while(dsrList[i].name[0] != 0 && request_break == 0) {
    cb = dsrList[i].crubase;
    bk_tputs_ram(bk_uint2hex(cb));
    tputs_rom(" -");
    if (addresses) {
      bk_tputc('\n');
    }
    while (cb == dsrList[i].crubase) {
      bk_tputc(' ');
      bk_tputs_ram(dsrList[i].name);
      if (addresses) {
        bk_tputc(':');
        bk_tputs_ram(bk_uint2hex(dsrList[i].addr));
        bk_tputc('\n');
      }
      i++;
    }
    bk_tputc('\n');
  }
}


