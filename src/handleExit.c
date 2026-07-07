#include "banks.h"

#define MYBANK BANK(0)

#include "commands.h"
#include "main.h"
#include "setupScreen.h"
#include "resetSams.h"

int handleExit() {
  bk_setupScreen(0);
  bk_resetSams();
  reboot();
  return 0;
}

