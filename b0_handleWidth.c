#include "banks.h"
#define MYBANK BANK(0)

#include "commands.h"

#include "b0_main.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b8_setupScreen.h"
#include <string.h>

void handleWidth() {
  char* tok = bk_strtok(0, ' ');
  int width = bk_atoi(tok);

  if (width == 80 && bk_isF18A()) {
    bk_setupScreen(80);
    return;
  }

  if (width == 40) {
    bk_setupScreen(40);
    return;
  }

  tputs_rom("no supported width specified\n");
}

