#include "banks.h"

#define MYBANK BANK(0)

#include "commands.h"
#include "b0_main.h"
#include "b8_setupScreen.h"
#include "b9_resetSams.h"

void handleExit() {
  bk_setupScreen(0);
  bk_resetSams();
  reboot();
}

