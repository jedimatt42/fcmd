#include "banks.h"

#define MYBANK BANK(0)

#include "commands.h"
#include "b1cp_terminal.h"

void handleCls() {
  char cls[] = { 27, '[', '2', 'J', 0 };
  tputs_rom(cls);
}

