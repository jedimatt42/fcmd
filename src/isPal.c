#include "banks.h"
#define MYBANK BANK(10)

#include <vdp.h>

int isPal() {
  volatile int t;

  // Wait for VDP interrupt with timeout
  t = 30000;
  __asm__(
    "limi 0\n\t"
    "clr r12\n"
    "0:\n\t"
    "dec %0\n\t"
    "jeq 2f\n\t"
    "tb 2\n\t"
    "jeq 0b\n\t"
    "movb @>8802,r12\n"
    "2:"
    : "+r"(t)
    :
    : "r12"
  );

  // If no interrupt occurred, it must be a misconfigured 9938 expansion.
  if (t == 0) return 0;

  // Normalize: wait for next VDP interrupt
  VDP_WAIT_VBLANK_CRU;

  // Start TMS9901 timer with max value (>3FFF = 16383)
  // Timer decrements every 64 PHI3* cycles = ~21.33 us
  __asm__(
    "clr r12\n\t"
    "sbo 0\n\t"
    "li r1,>3FFF\n\t"
    "inct r12\n\t"
    "ldcr r1,14\n\t"
    "dect r12\n\t"
    "sbz 0\n\t"
    : : : "r12", "r1"
  );

  // Wait for next VDP interrupt
  VDP_WAIT_VBLANK_CRU;

  // Read timer and stop it
  __asm__(
    "clr r12\n\t"
    "sbo 0\n\t"
    "stcr r1,15\n\t"
    "srl r1,1\n\t"
    "mov r1,%0\n\t"
    "ldcr r12,15\n\t"
    : "=r"(t)
    :
    : "r12", "r1"
  );

  // NTSC (60Hz): ~782 ticks elapsed, remaining ~15601
  // PAL  (50Hz): ~939 ticks elapsed, remaining ~15444
  return t < 15500;
}
