#include "banks.h"
#define MYBANK BANK_3

#include "commands.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include <conio.h>
#include <string.h>

void handleSet() {
  char* item = strtok(0, " ");
  if (!strcmpi(item, "COLOR")) {
    char* fore = strtok(0, " ");
    char* back = strtok(0, " ");
    int fc = atoi(fore);
    if (fc) {
      textcolor(fc);
    } else {
      tputs("must specify a foreground color 1-15\n");
      return;
    }
    int bg = atoi(back);
    if (bg) {
      bgcolor(bg);
      if (displayWidth == 80) {
        VDP_SET_REGISTER(VDP_REG_COL, bg & 0x0f);
      }
    }
  }
}