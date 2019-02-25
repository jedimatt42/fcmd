#include "banks.h"
#define MYBANK BANK_0

#include "b2_commands.h"
#include "b0_main.h"
#include "b1cp_strutil.h"
#include <string.h>
#include <conio.h>
#include <system.h>

extern void fg99();

extern char fg99_msg;

void handleFg99() {
  char* cart = strtok(0, " ");

  if (cart == 0) {
    cputs("error, no cartridge image name specified\n");
    return;
  }

  strncpy( &fg99_msg, cart, 8);
  strpad( &fg99_msg, 8, ' ');

  resetF18A();
  fg99();
}