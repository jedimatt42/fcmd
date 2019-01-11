#include "dsrutil.h"
#include "lvl2.h"
#include "strutil.h"
#include "oem.h"
#include "tifloat.h"
#include "main.h"
#include "commands.h"
#include "globals.h"
#include "parsing.h"
#include "getstr.h"

#include <string.h>
#include <conio.h>
#include <kscan.h>
#include <vdp.h>

#define TIPIMAN_VER "1.3"

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
  unlock_f18a();
  unsigned char testcode[6] = { 0x04, 0xE0, 0x3F, 0x00, 0x03, 0x40 };
  vdpmemcpy(0x3F00, testcode, 6);
  VDP_SET_REGISTER(0x36, 0x3F);
  VDP_SET_REGISTER(0x37, 0x00);
  return !vdpreadchar(0x3F00);
}

void resetF18A() {
  lock_f18a();
  set_graphics(0); // just to reset EVERYTHING
}

void setupScreen(int width) {
  resetF18A();
  bgcolor(COLOR_CYAN);
  textcolor(COLOR_BLACK);
  if (width == 80) {
    displayWidth = 80;
    set_text80_color();
  } else if(width == 40) {
    displayWidth = 40;
    set_text();
  }

  clrscr();
  gotoxy(0,23);
  defineChars();
}

void titleScreen() {
  cprintf("TIPIFM v%s\n", TIPIMAN_VER);
  cprintf("www.jedimatt42.com\n");
}

void main()
{
  initObject();
  setupScreen(isF18A() ? 80 : 40);
  titleScreen();
  loadDriveDSRs();
  currentDsr = dsrList;
  strcpy(currentPath, currentDsr->name);
  strcat(currentPath, ".");
  char buffer[256];
  buffer[0] = 0;
  defineChars();

  while(1) {
    VDP_INT_POLL;
    strset(buffer, 0, 255);
    cprintf("\n[%x.%s]\n$ ", currentDsr->crubase, currentPath);
    getstr(2, 23, buffer, displayWidth - 3, backspace);
    cprintf("\n");
    if (buffer[0] != 0) {
      handleCommand(buffer);
    }
  }
}

void onVolInfo(struct VolInfo* volInfo) {
  cprintf("Vol: %s\n", volInfo->volname);
  column = 0;
}

void onDirEntry(struct DirEntry* dirEntry) {
  gotoxy(column,23);
  cprintf("%s", dirEntry->name);
  gotoxy(column + 11,23);
  cprintf(ftypes[dirEntry->type - 1]);
  if (dirEntry->reclen != 0) {
    cprintf(" %d", dirEntry->reclen);
  }
  column += 20;
  if (column == displayWidth) {
    cprintf("\n");
    column = 0;
  }
}