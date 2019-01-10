#include "banking.h"
#include <conio.h>

// declare our current bank
#define MYBANK BANK_1

#include "third.h"

DECLARE_BANKED(third, BANK_2, void bk_third(), trampoline())

void second() {
  // even though constant, copy it into stack space for cross bank visibility
  const char msg[] = "Hello from >6002!";
  gotoxy(0,1);
  // cputs is in this bank, so no need to use the bank switch stub
  cputs(msg);
  bk_third();
}

