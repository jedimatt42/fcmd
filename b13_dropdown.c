#include "banks.h"
#define MYBANK BANK(13)

#include "b0_globals.h"
#include "b8_terminal.h"
#include "vdp.h"
#include "conio.h"
#include "b13_editor.h"


void dropDownBar(int y) {
  conio_x = 0;
  conio_y = y;
  ED_VDPCHAR(0xd4);
  for (int x = 1; x < displayWidth - 1; x++) {
    ED_VDPCHAR(0xcd);
  }
  ED_VDPCHAR(0xbe);
}

void dropDownSpace(int y) {
  conio_x = 0;
  conio_y = y;
  ED_VDPCHAR(0xb3);
  for (int x = 1; x < displayWidth - 1; x++) {
    ED_VDPCHAR(' ');
  }
  ED_VDPCHAR(0xb3);
}

void dropDown(int linecount) {
  for(int i=0; i<linecount; i++) {
    dropDownSpace(i);
  }
  dropDownBar(linecount);
}

