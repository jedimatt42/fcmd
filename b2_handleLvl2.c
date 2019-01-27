#include "banks.h"
#define MYBANK BANK_2

#include "b2_commands.h"
#include "b1_libti99.h"
#include "b2_dsrutil.h"
#include "b0_strutil.h"

void handleLvl2() {
  char* tok = bk_strtok(0, " ");
  int crubase = bk_htoi(tok);

  if (crubase == 0) {
    CCPUTS("no crubase specified\n");
    return;
  }

  enableROM(crubase);
  struct DeviceRomHeader* rom = (struct DeviceRomHeader*)0x4000;

  struct NameLink* link = rom->basiclnk;
  while(link != 0) {
    if (link->name[0] == 1) {
      CCPUTS(" >");
      bk_cputs(bk_uint2hex(link->name[1]));
    }
    link = link->next;
  }
  bk_cputc('\n');

  disableROM(crubase);
}

