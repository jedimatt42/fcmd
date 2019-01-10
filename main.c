#include "banking.h"
#include <conio.h>
#include <system.h>

// declare our current bank
#define MYBANK BANK_0

#include "second.h"

DECLARE_BANKED(set_text, BANK_1, void bk_set_text(), trampoline())
DECLARE_BANKED(charsetlc, BANK_1, void bk_charsetlc(), trampoline())
// this one passes arguments
DECLARE_BANKED(cputs, BANK_1, void bk_cputs(const char* s), trampoline(s))
DECLARE_BANKED(halt, BANK_1, void bk_halt(), trampoline())
DECLARE_BANKED(second, BANK_1, void bk_second(), trampoline())

void main() {
  bk_set_text();
  bk_charsetlc();

  // even though constant, copy it into stack space for cross bank visibility
  const char msg[] = "Hello from >6000!";
  gotoxy(0,0);
  bk_cputs(msg);

  bk_second();

  bk_halt();
}

