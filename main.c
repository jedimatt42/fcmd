#include "banking.h"
#include "counter.h"

// declare our current bank
#include "bank0.h"

#include <system.h>
#include <conio.h>

const char message[] = "more";

void main() {
  // setupScreen is in bank1, and depends on a bunch of libti99 stuff.
  // so things like vdp_*.o are linked into bank1 to reside next to the
  // primary calling code.
  BANK_CALL(setupScreen, BANK_1, void (*)(void))();

  // none-pointer values pass well. 
  // make a call to 'countup' in BANK_1 from BANK_0. with given signature.
  int a = 1;
  int b = 2;
  int c = 4;
  int d = BANK_CALL(countup, BANK_1, int (*)(int,int,int))(a, b, c);

  gotoxy(0,23);
  // DEMONSTRATES GOTCHA:  message is in bank0, so code in bank1
  // cannot access it. will print garbage. whatever is at that address
  // in bank1. 
  BANK_CALL(cputs, BANK_1, void (*)(const char*))(message);

  // I linked this one into bank0, sys_*.o, so I can call it directly.
  halt();

  // if a function needs to be visible to both banks, like the trampoline, then
  // it should end up in the common cart_rom .text section... gcc will make calls
  // to memcpy if creating a string on the stack, so that is a good candidate.
}

int second_data = 0xA55A;

