#include "banking.h"
#include "counter.h"

// declare our current bank
#include "bank0.h"

void main() {
  // make a call to 'countup' in BANK_1 from BANK_0. with given signature.
  BANK_CALL(countup, BANK_1, int (*)(void))();
}

