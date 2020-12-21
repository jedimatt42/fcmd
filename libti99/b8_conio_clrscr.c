#include <banks.h>

#define MYBANK BANK(0)

#include "conio.h"
#include <b0_globals.h>

extern unsigned int conio_scrnCol; // conio_bgcolor.c

void clrscr() {
  // use the conio functions for text modes
  gotoxy(0, nTitleLine);
  int limit = nTextEnd + 1;
  if (displayHeight == 26) {
    limit += displayWidth;
  }
  vdpmemset(conio_getvram(), ' ', limit);
  if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
    vdpmemset(gColor, conio_scrnCol, limit);
  }
}
