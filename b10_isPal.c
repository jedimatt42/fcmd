#include "banks.h"
#define MYBANK BANK(10)

int isPal() {
  volatile int cycles = 0;

  volatile int limit = 6000;

  // wait for VDP interrupt
  // with limit loop, in case it doesn't physically exist
  // EVPC card might not have VDP-INT hooked up...
  // (or it may be connected to EXT-INT tb 1)
  __asm__(
    "limi 0\n\t"
    "clr r12\n"
    "loop_%=:\n\t"
    "dec %0\n\t"
    "jeq break_%=\n\t"
    "tb 2\n\t"
    "jeq loop_%=\n"
    "break_%=:\n\t"
    "movb @>8802,r12"
    : "=r"(cycles)
    : "r"(cycles)
    : "r12"
  );

  if (cycles == 0) {
    return 0;
  }

  // again, provides a normalized result
  __asm__(
    "clr r12\n\t"
    "tb 2\n\t"
    "jeq -4\n\t"
    "movb @>8802,r12"
    : : : "r12"
  );

  cycles = 0;

  // now wait again, but count while we wait.
  // and repeat this 4 times, a fraction of a second
  // as it is not precise
  for (int i = 0; i < 6; i++) {
    __asm__(
      "clr r12\n\t"
      "inc %0\n\t"
      "tb 2\n\t"
      "jeq -6\n\t"
      "movb @>8802,r12"
      : "=r"(cycles)
      : "r"(cycles)
      : "r12"
    );
  }

  // with 6 iterations, NTSC reaches about 6800, and PAL reaches 8000
  return cycles >= 7500;
}
