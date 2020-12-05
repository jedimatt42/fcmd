#include "banks.h"

#define MYBANK BANK(3)

#include "commands.h"
#include <vdp.h>
#include <conio.h>
#include "b8_terminal.h"
#include "b8_setupScreen.h"
#include "b0_globals.h"
#include "b5_clock.h"

void handleBar() {
  nTitleLine = nTitleLine ? 0 : 1;
  bk_setupScreen(displayWidth);
  if (nTitleLine) {
    bk_drawBar();
    gotoxy(0, 1);
  }
}
