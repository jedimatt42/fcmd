#include "banking.h"
#include <conio.h>

// declare our current bank
#define MYBANK BANK_2

#include "fourth.h"

DECLARE_BANKED(cputs, BANK_1, void bk_cputs(const char* s), trampoline(s))
DECLARE_BANKED(fourth, BANK_3, void bk_fourth(), trampoline())

void third() {
  // even though constant, copy it into stack space for cross bank visibility
  const char msg[] = "Hello from >6004!";
  gotoxy(0,2);
  bk_cputs(msg);
  bk_fourth();
}

