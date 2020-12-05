#include "banks.h"
#define MYBANK BANK(8)

#include "b8_setupScreen.h"
#include "b3_oem.h"
#include "b8_terminal.h"
#include "b0_globals.h"
#include "b10_detect_vdp.h"
#include <vdp.h>
#include <conio.h>

int isF18A() {
  unlock_f18a();
  unsigned char testcode[6] = { 0x04, 0xE0, 0x3F, 0x00, 0x03, 0x40 };
  vdpmemcpy(0x3F00, testcode, 6);
  {
    VDP_SET_REGISTER(0x36, 0x3F);
    VDP_SET_REGISTER(0x37, 0x00);
  }

  int frames = 3;
  while(frames--) {
    VDP_SET_ADDRESS(0x3F00);
    int res = VDPRD;
    if (!res) {
      return 1;
    }
  }

  return 0;
}

void setupScreen(int width) {
  lock_f18a();
  if (width == 0) {
    set_graphics(0);
    // to override some VDP registers for EA5 compatibility
    // VDP_SET_REGISTER(VDP_REG_MODE0, 0x00);
    VDP_SET_REGISTER(VDP_REG_MODE1, 0xE0);
    // VDP_SET_REGISTER(VDP_REG_SIT, 0x00);
    // VDP_SET_REGISTER(VDP_REG_CT, 0x0E);
    // VDP_SET_REGISTER(VDP_REG_PDT, 0x01);
    // VDP_SET_REGISTER(VDP_REG_SAL, 0x06);
    VDP_SET_REGISTER(VDP_REG_SDT, 0x00);
    VDP_SET_REGISTER(VDP_REG_COL, 0xF3);

    // erase first 4k of vdp
    vdpmemset(0, 0, 4192);
    // TODO load ea copyright and cursor

    // set colors
    vdpmemset(0x0380, 0x13, 32);
    clrscr();
    charset();
    return;
  } else if (width == 80) {
    displayWidth = 80;
    if (vdp_type == VDP_F18A) {
      set_text80x30_color();
      displayHeight = 30;
    } else {
      set_text80();
      displayHeight = 26;
    }
  } else { // 40 is the only other allowed value.
    displayWidth = 40;
    set_text();
    displayHeight = 24;
  }
  bk_initTerminal();

  if (displayWidth == 80 && vdp_type == VDP_F18A) {
    bgcolor(background);
    textcolor(foreground);
    VDP_SET_REGISTER(VDP_REG_COL, background & 0x0f);
  } else {
    VDP_SET_REGISTER(VDP_REG_COL, foreground << 4 | background);
  }

  clrscr();
  gotoxy(0,nTitleLine);
  bk_defineChars();
}


