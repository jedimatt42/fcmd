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
#include "b3_banner.h"
#include <sound.h>
#include <string.h>
#include <vdp.h>
#include <conio.h>
#include <kscan.h>
#include "b1cp_terminal.h"
#include "b4_labellist.h"

#define APP_VER "0.9"

char commandbuf[256];

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

void resetF18A() {
  lock_f18a();
}

void setupScreen(int width) {
  resetF18A();
  if (width == 80) {
    displayWidth = 80;
    set_text80_color();
  } else { // 40 is the only other allowed value.
    displayWidth = 40;
    set_text();
  }
  initTerminal();
  termWidth = displayWidth;

  if (termWidth == 80) {
    bgcolor(background);
    textcolor(foreground);
    VDP_SET_REGISTER(VDP_REG_COL, background & 0x0f);
  } else {
    VDP_SET_REGISTER(VDP_REG_COL, foreground << 4 | background);
  }

  clrscr();
  gotoxy(0,0);
  bk_defineChars();
}

void titleScreen() {
  tputs("Force Command v");
  tputs(APP_VER);
  tputc(' ');
  tputs(__DATE__);
  tputs("\nwww.jedimatt42.com\n\n");
}

void main()
{
  bk_libtoram();
  MUTE_SOUND();

  foreground = 15;
  background = 4;
  setupScreen(isF18A() ? 80 : 40);

  bk_loadDriveDSRs();
  scripton = 0;

  char autocmd[13];
  strcpy(autocmd, currentPath);
  strcat(autocmd, "AUTOCMD");
  struct DeviceServiceRoutine* autodsr = currentDsr;
  int ranauto = runScript(autodsr, autocmd);

  if (!ranauto) {
    if (displayWidth == 80) {
      bk_banner();
    }
    titleScreen();
    playtipi();
  }

  while(1) {
    VDP_INT_POLL;
    strset(commandbuf, 0, 255);
    tputc('[');
    tputs(uint2hex(currentDsr->crubase));
    tputc('.');
    tputs(currentPath);
    tputs("]\n$ ");
    getstr(2, conio_y, commandbuf, displayWidth - 3, backspace);
    tputs("\n");
    enable_more();
    handleCommand(commandbuf);
  }
}

int runScript(struct DeviceServiceRoutine* dsr, char* scriptName) {
  disable_more();
  int ran = 0;
  struct DeviceServiceRoutine* oldDsr = scriptDsr;
  scriptDsr = dsr;
  struct PAB pab;
  scriptPab = &pab;
  bk_labels_clear();
  scripton++;
  lineno = 0;

  int ferr = bk_dsr_open(dsr, &pab, scriptName, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL, 0);
  if (!ferr) {
    ran = 1;
    while(!ferr) {
      VDP_INT_POLL;
      strset(commandbuf, 0, 255);
      ferr = bk_dsr_read(dsr, &pab, 0);
      char k = kscan(5);
      if (k == 131 || k == 2) { // control-c or alt-4
        ferr = 1; // so abort script
      }
      if (!ferr) {
        lineno++;
        vdpmemread(pab.VDPBuffer, commandbuf, pab.CharCount);
        int l = strlen(commandbuf);
        // TI-Writer adds \r to lines, so erase those automatically if at end of line.
        if (commandbuf[l-1] == 13) {
          commandbuf[l-1] = 0;
        }
        handleCommand(commandbuf);
      }
    }
    bk_dsr_close(dsr, &pab);
  }
  scripton--;
  return ran;
}