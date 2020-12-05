#include "banks.h"
#define MYBANK BANK(5)

#include "b5_clock.h"
#include <vdp.h>

// incremented and managed in cgetc
static unsigned int counter_a = 0;


void clock_hook() {
  if (nTitleLine) {
    counter_a++;
    if (counter_a == 0x5000) {
      drawBar();
      counter_a = 0;
    }
  }
}
