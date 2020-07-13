#include "banks.h"
#define MYBANK BANK(3)

#include <conio.h>
#include "b1cp_terminal.h"

extern char bandata;

void banner() {
  tputs_rom(&bandata);
  tputs_rom("\n");
  bgcolor(4);
  textcolor(15);
}

