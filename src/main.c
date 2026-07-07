#include "banks.h"
#define MYBANK BANK(0)

#include "debugwait.h"
#include "main.h"

#include <sound.h>
#include <string.h>
#include <vdp.h>
#include <conio.h>
#include <kscan.h>
#include "parsing.h"
#include "globals.h"
#include "strutil.h"
#include "dsrutil.h"
#include "oem.h"
#include "banner.h"
#include "labellist.h"
#include "dictionary.h"
#include "heap.h"
#include "terminal.h"
#include "setupScreen.h"
#include "isPal.h"
#include "detect_vdp.h"
#include "history.h"
#include "clock.h"
#include "variables.h"
#include "prompt.h"
#include "palette.h"
#include "runScript.h"
#include "getstr.h"
#include "ed.h"

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
#if CONSOLE_ROM == BUILD_CONSOLE  
  tputs_rom("\n16-bit ROM\n");
#else
  tputs_rom("\n8-bit CART\n");
#endif
  tputs_rom("jedimatt42.com\n");
}

void prompt() {
  char* prompt_pattern = bk_vars_get(str2ram("PROMPT"));
  if (prompt_pattern != (char*) -1) {
    bk_print_prompt(prompt_pattern);
  } else {
    bk_print_prompt(str2ram("[\\c\\p]\\n$\\s"));
  }
}

void checkBackspaceVar() {
  char* bs_var = bk_vars_get(str2ram("BACKSPACE"));
  if (bs_var != (char*) -1) {
    if (0 == bk_strcmpi(str2ram("true"), bs_var)) {
      backspace = 1;
    } else if (0 == bk_strcmpi(str2ram("false"), bs_var)) {
      backspace = 0;
    }
  }
}

void cartmain()
{
  MUTE_SOUND();

  // stub so early var fetching will fail gently
  system_dict.addr = 0;
  system_dict.end = 0;

  vdp_type = bk_detect_vdp();
  bk_textcolor(15);
  bk_bgcolor(4);
  bk_setupScreen(vdp_type == VDP_9918 ? 40 : 80);
  if (vdp_type == VDP_F18A) {
    bk_set_palette(0, 0);
  }
  pal = bk_isPal();

  bk_loadDriveDSRs();

  bk_dict_init(&system_dict, heap_end);

  bk_history_init();

  bk_detectClock();

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
    //playtipi();
  }

  while(1) {
    checkBackspaceVar();
    char commandbuf[256];

    VDP_INT_POLL;
    bk_strset(commandbuf, 0, 255);
    prompt();
    history_on = 1;
    bk_commandLineEd(commandbuf, 256);
    bk_tputc('\n');
    history_on = 0;
    bk_handleCommand(commandbuf);
    if (nTitleLine) {
      bk_drawBar();
    }
  }
}
