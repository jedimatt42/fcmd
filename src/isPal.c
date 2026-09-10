#include "banks.h"
#define MYBANK BANK(10)

#include <vdp.h>
/*
#include "string.h"
#include "terminal.h"
*/

int isPal() {
  int t;

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

  // Let it run for 10 frames so the NTSC/PAL difference grows ~10x
  for (int i = 0; i < 10; i++) VDP_WAIT_VBLANK_CRU;

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

/*
  tputs_rom("\nt=");
  bk_tputs_ram(bk_uint2str((unsigned int)t));
  tputs_rom("\n");
*/

  // 10 frames: NTSC ~8563 remaining, PAL ~7003 remaining; threshold 7700
  return t < 7700;
}
