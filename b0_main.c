#include "banks.h"
#define MYBANK BANK_0

#include "b0_main.h"

#include "b0_getstr.h"
#include "b0_parsing.h"
#include "b0_globals.h"
#include "b1_libtoram.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include "b3_oem.h"
#include <sound.h>
#include <string.h>
#include <vdp.h>
#include <conio.h>

#define APP_VER "1.0"

unsigned int displayWidth = 40;
unsigned int column = 0;

const char tipibeeps[] = {
  0x04, 0x9f, 0xbf, 0xdf, 0xff, 0x02,
  0x03, 0x80, 0x05, 0x94, 0x07,
  0x03, 0x8B, 0x06, 0x94, 0x07,
  0x03, 0x80, 0x05, 0x90, 0x0e,
  0x01, 0x9f, 0x00
};

void playtipi() {
  char* pSrc = (char*) tipibeeps;
  unsigned int cnt = sizeof(tipibeeps);
  VDP_SET_ADDRESS_WRITE(0x3E00);
  while (cnt--) {
    VDPWD=*(pSrc++);
  }
  SET_SOUND_PTR(0x3E00);
  SET_SOUND_VDP();
  START_SOUND();
  while(SOUND_CNT != 0) {
    VDP_INT_POLL;
  }
  MUTE_SOUND();
  VDP_INT_POLL;
}

int isF18A() {
  unlock_f18a();
  unsigned char testcode[6] = { 0x04, 0xE0, 0x3F, 0x00, 0x03, 0x40 };
  vdpmemcpy(0x3F00, testcode, 6);
  {
    VDP_SET_REGISTER(0x36, 0x3F);
    VDP_SET_REGISTER(0x37, 0x00);
  }
  VDP_SET_ADDRESS(0x3F00);
  __asm__("NOP");
  int res = VDPRD;
  return !res;
}

void resetF18A() {
  lock_f18a();
}

void setupScreen(int width) {
  resetF18A();
  bgcolor(COLOR_CYAN);
  textcolor(COLOR_BLACK);
  if (width == 80) {
    displayWidth = 80;
    set_text80_color();
  } else { // 40 is the only other allowed value.
    displayWidth = 40;
    set_text();
  }

  clrscr();
  gotoxy(0,23);
  bk_defineChars();
}

void titleScreen() {
  cputs("TIPICMD v");
  cputs(APP_VER);
  cputs("\nwww.jedimatt42.com\n");
}

void main()
{
  bk_libtoram();
  setupScreen(isF18A() ? 80 : 40);
  bk_defineChars();
  titleScreen();
  playtipi();

  bk_loadDriveDSRs();

  char buffer[256];

  while(1) {
    VDP_INT_POLL;
    strset(buffer, 0, 255);
    cputc('[');
    cputs(uint2hex(currentDsr->crubase));
    cputc('.');
    cputs(currentPath);
    cputs("]\n$ ");
    getstr(2, conio_y, buffer, displayWidth - 3, backspace);
    cputs("\n");
    handleCommand(buffer);
  }
}
