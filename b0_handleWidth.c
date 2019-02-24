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

  if (width == 80 && isF18A()) {
    setupScreen(80);
    return;
  }

  if (width == 40) {
    setupScreen(40);
    return;
  }
  
  cputs("no supported width specified\n");
}

