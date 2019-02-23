#include "banks.h"
#define MYBANK BANK_0

#include "b2_commands.h"

#include "b0_main.h"
#include "b1cp_strutil.h"
#include <conio.h>
#include <string.h>

void handleWidth() {
  char* tok = strtok(0, " ");
  int width = atoi(tok);

  if (width == 40 || width == 80) {
    setupScreen(width);
  } else {
    cputs("no width specified\n");
  }
}

