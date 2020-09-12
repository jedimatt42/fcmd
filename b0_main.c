#include "banks.h"
#define MYBANK BANK(0)

#include "b0_main.h"

#include <sound.h>
#include <string.h>
#include <vdp.h>
#include <conio.h>
#include <kscan.h>
#include "b0_parsing.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b3_oem.h"
#include "b3_banner.h"
#include "b4_labellist.h"
#include "b8_terminal.h"
#include "b8_getstr.h"
#include "b8_setupScreen.h"

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

void titleScreen() {
  tputs_rom("Force Command v");
  tputs_rom(APP_VER);
  bk_tputc(' ');
  tputs_rom(__DATE__);
  tputs_rom("\nwww.jedimatt42.com\n\n");
}

void main()
{
  MUTE_SOUND();

  foreground = 15;
  background = 4;
  bk_setupScreen(bk_isF18A() ? 80 : 40);

  bk_loadDriveDSRs();
  scripton = 0;

  char autocmd[13];
  bk_strcpy(autocmd, currentPath);
  bk_strcat(autocmd, str2ram("AUTOCMD"));
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
    char commandbuf[256];

    VDP_INT_POLL;
    bk_strset(commandbuf, 0, 255);
    bk_tputc('[');
    bk_tputs_ram(bk_uint2hex(currentDsr->crubase));
    bk_tputc('.');
    bk_tputs_ram(currentPath);
    tputs_rom("]\n$ ");
    bk_getstr(commandbuf, displayWidth - 3, backspace);
    bk_tputc('\n');
    bk_enable_more();
    handleCommand(commandbuf);
  }
}

int runScript(struct DeviceServiceRoutine* dsr, char* scriptName) {
  bk_disable_more();
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
      char commandbuf[256];

      bk_strset(commandbuf, 0, 255);
      ferr = bk_dsr_read(dsr, &pab, 0);
      char k = bk_kscan(5);
      if (k == 131 || k == 2) { // control-c or alt-4
        ferr = 1; // so abort script
      }
      if (!ferr) {
        lineno++;
        vdpmemread(pab.VDPBuffer, commandbuf, pab.CharCount);
        int l = bk_strlen(commandbuf);
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
