#include "banks.h"

#define MYBANK BANK(3)

#include "commands.h"
#include <vdp.h>
#include <conio.h>
#include "terminal.h"
#include "setupScreen.h"
#include "globals.h"
#include "clock.h"

static int error() {
  tputs_rom("error, on or off argument missing.\n");
  return 0;
}

int handleBar() {
  char* tok = bk_strtok(0, ' ');
  if (!tok) {
    error();
  return 0;
  }

  if (0 == bk_strcmpi(tok, str2ram("on"))) {
    nTitleLine = 1;
  } else if (0 == bk_strcmpi(tok, str2ram("off"))) {
    nTitleLine = 0;
  } else {
    error();
  return 0;
  }

  bk_setupScreen(displayWidth);
  if (nTitleLine) {
    bk_drawBar();
    gotoxy(0, 1);
  }
  return 0;
}
