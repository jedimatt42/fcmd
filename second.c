#include "banking.h"
#include <conio.h>

// declare our current bank
#define MYBANK BANK_1

#include "third.h"

DECLARE_BANKED(cputs, BANK_1, void bk_cputs(const char* s), trampoline(s))
DECLARE_BANKED(third, BANK_2, void bk_third(), trampoline())

void second() {
  // even though constant, copy it into stack space for cross bank visibility
  const char msg[] = "Hello from >6002!";
  gotoxy(0,1);
  bk_cputs(msg);
  bk_third();
}

