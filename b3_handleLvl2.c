#include "banks.h"
#define MYBANK BANK(3)

#include "commands.h"
#include "b2_dsrutil.h"
#include "b1cp_strutil.h"
#include <string.h>
#include "b1cp_terminal.h"

void handleLvl2() {
  char* tok = strtok(0, " ");
  int crubase = htoi(tok);

  if (crubase == 0) {
    tputs_rom("no crubase specified\n");
    return;
  }

  bk_enableROM(crubase);
  struct DeviceRomHeader* rom = (struct DeviceRomHeader*)0x4000;

  struct NameLink* link = rom->basiclnk;
  while(link != 0) {
    if (link->name[0] == 1) {
      tputs_rom(" >");
      register unsigned int opname = 0;
      opname = link->name[1];
      tputs_ram(uint2hex(opname)+2);
    }
    link = link->next;
  }
  tputc('\n');

  bk_disableROM(crubase);
}

