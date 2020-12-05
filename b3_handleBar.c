#include "banks.h"

#define MYBANK BANK(3)

#include "commands.h"
#include <vdp.h>
#include <conio.h>
#include "b8_terminal.h"
#include "b8_setupScreen.h"
#include "b0_globals.h"
#include "b5_clock.h"

static void error() {
  tputs_rom("error, on or off argument missing.\n");
}

void handleBar() {
  char* tok = bk_strtok(0, ' ');
  if (!tok) {
    error();
    return;
  }

  if (0 == bk_strcmpi(tok, str2ram("on"))) {
    nTitleLine = 1;
  } else if (0 == bk_strcmpi(tok, str2ram("off"))) {
    nTitleLine = 0;
  } else {
    error();
    return;
  }

  bk_setupScreen(displayWidth);
  if (nTitleLine) {
    bk_drawBar();
    gotoxy(0, 1);
  }
}
