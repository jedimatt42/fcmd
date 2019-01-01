#include "banking.h"
#include "counter.h"

// declare our current bank
#include "bank0.h"

#define PORT ((volatile int*)0x8320)

void main() {
  // make a call to 'countup' in BANK_1 from BANK_0. with given signature.
  int a = 1;
  int b = 2;
  int c = 4;
  int d = BANK_CALL(countup, BANK_1, int (*)(int,int,int))(a, b, c);
  *PORT = d;
}

int second_data = 0xA55A;

