#include "banking.h"
#include "counter.h"

// declare our current bank
#include "bank0.h"

#include <system.h>
#include <conio.h>

DECLARE_BANKED(setupScreen, BANK_1, void far_setupScreen(), trampoline())
DECLARE_BANKED(countup, BANK_1, int far_countup(int a, int b, int c), return (int) trampoline(a, b, c))
DECLARE_BANKED(cputs, BANK_1, void far_cputs(const char* s), trampoline(s))
DECLARE_BANKED(cputc, BANK_1, void far_cputc(int ch), trampoline(ch))
DECLARE_BANKED(checkresult, BANK_1, void far_checkresult(int s, int ex), trampoline(s, ex))

DECLARE_BANKED(test1, BANK_1, void far_test1(), trampoline())

const char message[] = "more";

void main() {

  // setupScreen is in bank1, and depends on a bunch of libti99 stuff.
  // so things like vdp_*.o are linked into bank1 to reside next to the
  // primary calling code.
  far_setupScreen();

  // none-pointer values pass well. 
  // make a call to 'countup' in BANK_1 from BANK_0. with given signature.
  int a = 1;
  int b = 2;
  int c = 4;
  int d = far_countup(a, b, c);
  d += far_countup(a, b, d);
  far_checkresult(d, 17); // check that d is some expected number, print something about it.


  // bank chaining : will go from bank 0, to bank 1, to bank 0, and then rewind
  far_cputc('a');    
  far_test1();
  far_cputc('A');

  // I linked this one into bank0, sys_*.o, so I can call it directly.
  halt();

  // if a function needs to be visible to both banks, like the trampoline, then
  // it should end up in the common cart_rom .text section... gcc will make calls
  // to memcpy if creating a string on the stack, so that is a good candidate.

  // DEMONSTRATES GOTCHA:  message is in bank0, so code in bank1
  // cannot access it. will print garbage. whatever is at that address
  // in bank1. 
  far_cputs(message);
}

void test2() {
  far_cputc('a');
}

