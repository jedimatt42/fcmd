#include "banks.h"
#define MYBANK BANK(0)

#include "commands.h"
#include "b0_main.h"
#include "b1_strutil.h"
#include <string.h>
#include "b8_terminal.h"
#include "b8_setupScreen.h"
#include <system.h>

extern void fg99();

extern char fg99_msg;

void handleFg99() {
  char* cart = bk_strtok(0, ' ');

  if (cart == 0) {
    tputs_rom("error, no cartridge image name specified\n");
    return;
  }

  if (bk_strlen(cart) > 8) {
    tputs_rom("error, name too long\n");
    return;
  }

  bk_strcpy(&fg99_msg, cart);

  bk_setupScreen(0);
  fg99();
}