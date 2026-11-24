#include "banks.h"
#define MYBANK BANK(8)

#include "setupScreen.h"
#include "oem.h"
#include "terminal.h"
#include "globals.h"
#include "detect_vdp.h"
#include "loadFont.h"
#include <vdp.h>
#include <conio.h>

static void setupText40(int height) {
  set_text();

  if (height == 30 && vdp_type == VDP_F18A) {
    unlock_f18a();
    VDP_SET_REGISTER(0x31, 0x40); // row 30 mode
    VDP_SET_REGISTER(0x32, 0x00); // global colors
    nTextRow = 29 * 40;
    nTextEnd = (30 * 40) - 1;
    nTextPos = nTextRow;
    nTextFlags = TEXT_FLAG_IS_F18A | TEXT_WIDTH_40 | TEXT_HEIGHT_30;
    displayHeight = 30;
  } else if (height == 26 &&
             (vdp_type == VDP_9938 || vdp_type == VDP_9958)) {
    VDP_SET_REGISTER(0x09, 0x80); // LN=1, 26.5 rows
    nTextRow = 25 * 40;
    nTextEnd = (26 * 40) - 1;
    nTextPos = nTextRow;
    nTextFlags = TEXT_WIDTH_40 | TEXT_HEIGHT_26;
    displayHeight = 26;
  } else {
    if (vdp_type == VDP_F18A) {
      VDP_SET_REGISTER(0x31, 0x00); // row 30 off
      VDP_SET_REGISTER(0x32, 0x00); // attributes off
    }
    nTextFlags = TEXT_WIDTH_40 | TEXT_HEIGHT_24;
    displayHeight = 24;
  }
}

static void setupText80(int height) {
  if (vdp_type == VDP_F18A) {
    if (height == 30) {
      set_text80_color(30);
      displayHeight = 30;
    } else {
      set_text80_color(24);
      displayHeight = 24;
    }
  } else {
    if (height == 26) {
      set_text80_height(26);
      displayHeight = 26;
    } else {
      set_text80_height(24);
      displayHeight = 24;
    }
  }
}

void setupScreenEx(int width, int height) {
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
  }

  if (width == 80 && vdp_type != VDP_9918) {
    displayWidth = 80;
    setupText80(height);
  } else {
    displayWidth = 40;
    setupText40(height);
  }

  bk_initTerminal();

  int foreground = FOREGROUND;
  int background = BACKGROUND;
  if (displayWidth == 80 && vdp_type == VDP_F18A) {
    term_set_bg_color(background);
    term_set_text_color(foreground);
    VDP_SET_REGISTER(VDP_REG_COL, background);
  } else {
    VDP_SET_REGISTER(VDP_REG_COL, foreground << 4 | background);
  }

  clrscr();
  gotoxy(0,nTitleLine);
  if (bk_load_font()) {
    // if font file fails to load, then use ROM font
    bk_defineChars();
  }
}

void setupScreen(int width) {
  int height = 24;
  if (width == 80) {
    height = (vdp_type == VDP_F18A) ? 24 : 26;
  }
  setupScreenEx(width, height);
}


