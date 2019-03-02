#include "banks.h"

#define MYBANK BANK_0

#include "commands.h"
#include "b0_main.h"

void handleExit() {
  resetF18A();

  reboot();
}

