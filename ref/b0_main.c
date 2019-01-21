#include "banks.h"
#define MYBANK BANK_0

#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b0_strutil.h"
#include "b3_oem.h"
#include "b1_tifloat.h"
#include "b0_main.h"
#include "b2_commands.h"
#include "b0_globals.h"
#include "b0_parsing.h"
#include "b0_getstr.h"

#include "b1_libti99.h"

#define APP_VER "1.3"

const char* const ftypes[] = {
  "D/F",
  "D/V",
  "I/F",
  "I/V",
  "PRG",
  "DIR"
};

int displayWidth;
int column;

void initObject() {
  displayWidth = 40;
  column = 0;
  backspace = 0;
}

void sleep(int jiffies) {
  for(int i=0; i<jiffies;i++) {
    VDP_WAIT_VBLANK_CRU;
  }
}

int isF18A() {
  bk_unlock_f18a();
  unsigned char testcode[6] = { 0x04, 0xE0, 0x3F, 0x00, 0x03, 0x40 };
  bk_vdpmemcpy(0x3F00, testcode, 6);
  VDP_SET_REGISTER(0x36, 0x3F);
  VDP_SET_REGISTER(0x37, 0x00);
  return !bk_vdpreadchar(0x3F00);
}

void resetF18A() {
  bk_lock_f18a();
  bk_set_graphics(0); // just to reset EVERYTHING
}

void setupScreen(int width) {
  resetF18A();
  bk_bgcolor(COLOR_CYAN);
  bk_textcolor(COLOR_BLACK);
  if (width == 80) {
    displayWidth = 80;
    bk_set_text80_color();
  } else if(width == 40) {
    displayWidth = 40;
    bk_set_text();
  }

  bk_clrscr();
  gotoxy(0,23);
  bk_defineChars();
}

void titleScreen() {
  bk_cprintf("TIPIFM v%s\n", APP_VER);
  bk_cprintf("www.jedimatt42.com\n");
}

void main()
{
  initObject();
  setupScreen(isF18A() ? 80 : 40);
  titleScreen();
  bk_loadDriveDSRs();
  currentDsr = dsrList;
  bk_strcpy(currentPath, currentDsr->name);
  strcat(currentPath, ".");
  char buffer[256];
  buffer[0] = 0;
  bk_defineChars();

  while(1) {
    VDP_INT_POLL;
    strset(buffer, 0, 255);
    bk_cprintf("\n[%x.%s]\n$ ", currentDsr->crubase, currentPath);
    getstr(2, 23, buffer, displayWidth - 3, backspace);
    bk_cprintf("\n");
    if (buffer[0] != 0) {
      handleCommand(buffer);
    }
  }
}

void onVolInfo(struct VolInfo* volInfo) {
  bk_cprintf("Vol: %s\n", volInfo->volname);
  column = 0;
}

void onDirEntry(struct DirEntry* dirEntry) {
  gotoxy(column,23);
  bk_cprintf("%s", dirEntry->name);
  gotoxy(column + 11,23);
  bk_cprintf(ftypes[dirEntry->type - 1]);
  if (dirEntry->reclen != 0) {
    bk_cprintf(" %d", dirEntry->reclen);
  }
  column += 20;
  if (column == displayWidth) {
    bk_cprintf("\n");
    column = 0;
  }
}