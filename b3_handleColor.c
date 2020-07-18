#include "banks.h"
#define MYBANK BANK(3)

#include "commands.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include <conio.h>
#include <string.h>

void handleColor() {
  char* fore = strtok(0, " ");
  char* back = strtok(0, " ");
  int fc = atoi(fore);
  int oldfc = foreground;
  if (fore && fc >= 0 && fc <= 15) {
    foreground = fc;
  } else {
    tputs_rom("must specify a foreground color 0-15\n");
    return;
  }
  if (back) {
    int bg = atoi(back);
    if (back && bg >= 0 && bg <= 15) {
      background = bg;
    } else {
      tputs_rom("background must be between 0-15 if specified\n");
      foreground = oldfc;
      return;
    }
  }

  if (termWidth == 80) {
    bgcolor(background);
    textcolor(foreground);
    VDP_SET_REGISTER(VDP_REG_COL, background & 0x0f);
  } else {
    VDP_SET_REGISTER(VDP_REG_COL, foreground << 4 | background);
  }
}