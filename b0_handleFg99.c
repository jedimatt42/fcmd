#include "banks.h"
#define MYBANK BANK(0)

#include "commands.h"
#include "b0_main.h"
#include "b1_strutil.h"
#include <string.h>
#include "b8_terminal.h"
#include "b8_setupScreen.h"
#include "b1_fg99.h"
#include <system.h>

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

  char fg99_msg[20];
  fg99_msg[0] = 0x99;
  bk_strcpy(fg99_msg+1,str2ram("OKFG99"));
  fg99_msg[7] = 0x99;

  bk_strcpy(fg99_msg+8, cart);
  for(int i = 16; i<20; i++) {
    fg99_msg[i] = 0x00;
  }

  bk_setupScreen(0);
  bk_fg99(fg99_msg, 0x0000);
}