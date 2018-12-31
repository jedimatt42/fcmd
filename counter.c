#include "counter.h"

int countup() {
  static int counter = 0xCAFE;
  counter++;
  return counter;
}

