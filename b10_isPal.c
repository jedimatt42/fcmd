#include "banks.h"
#define MYBANK BANK(10)

// I see 5450 for PAL, and 4541 for NTSC, split the difference
#define PAL_THRESHOLD 0x0B00

int /*__attribute__((optimize("-O0")))*/ isPal() {
  register int cycles = 0;

  // wait for VDP interrupt
  __asm__(
      "limi 0\n\t"
      "clr r12\n\t"
      "tb 2\n\t"
      "jeq -4\n\t"
      "movb @>8802,r12"
      :
      :
      : "r12");

  // again, provides a normalized result
  __asm__(
      "clr r12\n\t"
      "tb 2\n\t"
      "jeq -4\n\t"
      "movb @>8802,r12"
      :
      :
      : "r12");

  // now wait again, but count while we wait.
  // and repeat this 4 times, a fraction of a second
  // as it is not precise
  for (int i = 0; i < 4; i++) {
    __asm__(
        "clr r12\n\t"
        "inc %0\n\t"
        "tb 2\n\t"
        "jeq -6\n\t"
        "movb @>8802,r12"
        : "=r"(cycles)
        : "r"(cycles)
        : "r12");
  }
  return cycles >= PAL_THRESHOLD;
}
