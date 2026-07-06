#include "banks.h"

#define MYBANK BANK(0)

#include "commands.h"
#include "b8_terminal.h"

int handleCls() {
  char cls[] = { 27, '[', '2', 'J', 0 };
  tputs_rom(cls);
  return 0;
}

