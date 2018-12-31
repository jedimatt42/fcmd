#include "banking.h"
#include "counter.h"
#include "bank0.h"

void main() {
  FAR_CALL(countup, BANK_1, int (*)(void))();
}

