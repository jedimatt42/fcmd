#include "banks.h"
#define MYBANK BANK_2

#include "commands.h"
#include "b2_dsrutil.h"
#include "b1cp_strutil.h"
#include <string.h>
#include <conio.h>

void handleLvl2() {
  char* tok = strtok(0, " ");
  int crubase = htoi(tok);

  if (crubase == 0) {
    cputs("no crubase specified\n");
    return;
  }

  enableROM(crubase);
  struct DeviceRomHeader* rom = (struct DeviceRomHeader*)0x4000;

  struct NameLink* link = rom->basiclnk;
  while(link != 0) {
    if (link->name[0] == 1) {
      cputs(" >");
      cputs(uint2hex(link->name[1]));
    }
    link = link->next;
  }
  cputc('\n');

  disableROM(crubase);
}

