#include "banks.h"
#define MYBANK BANK_0

#include "b2_commands.h"

#include "b0_main.h"
#include "b1_libti99.h"
#include "b0_strutil.h"

void handleWidth() {
  char* tok = strtok(0, " ");
  int width = bk_atoi(tok);

  if (width == 40 || width == 80) {
    setupScreen(width);
  } else {
    CCPUTS("no width specified\n");
  }
}

