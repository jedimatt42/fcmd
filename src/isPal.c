#include "banks.h"
#define MYBANK BANK(10)

#include <vdp.h>
#include "string.h"
#include "terminal.h"

int isPal() {
  int t;

  // Normalize: wait for next VDP interrupt
  VDP_WAIT_VBLANK;

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
  VDP_WAIT_VBLANK;

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

  tputs_rom("\nt=");
  bk_tputs_ram(bk_uint2str((unsigned int)t));
  tputs_rom("\n");

  // NTSC (60Hz): ~782 ticks elapsed, remaining ~15601
  // PAL  (50Hz): ~939 ticks elapsed, remaining ~15444
  // However in practice there is a delay gettting the timer started, so actual elapsed ticks will be less.
  // On MAME with evpc 9938 set to PAL, I measure 15722 remaining, and in NTSC I measure 16300.
  return t < 16000;
}
