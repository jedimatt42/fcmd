#include "banks.h"
#define MYBANK BANK(3)

#include "vdp_evpc.h"
#include "detect_vdp.h"
#include <vdp.h>

/*
 * EVPC power-up register table. The EVPC writes R0 separately and then this
 * table for R1-R19 and R23. R20-R22 (color burst), R24-R31 (unused) and the
 * V9938 command registers are deliberately left alone.
 */
static const unsigned char evpc_regs[] = {
  0x00, 0x00, /* R0  mode 0 */
  0x01, 0x20, /* R1  IE0, display blank, Graphics I */
  0x02, 0x00, /* R2  name table at >0000 */
  0x03, 0x0E, /* R3  color table at >0380 */
  0x04, 0x01, /* R4  pattern table at >0800 */
  0x05, 0x06, /* R5  sprite attributes at >0300 */
  0x06, 0x00, /* R6  sprite patterns at >0000 */
  0x07, 0xF7, /* R7  white on cyan */
  0x08, 0x08, /* R8  64K/128K VRAM */
  0x09, 0x02, /* R9  PAL timing */
  0x0A, 0x00, /* R10 color table high bits */
  0x0B, 0x00, /* R11 sprite attributes high bits */
  0x0C, 0xF7, /* R12 blink text/background colors */
  0x0D, 0x00, /* R13 blink period */
  0x0E, 0x00, /* R14 VRAM access page 0 */
  0x0F, 0x00, /* R15 status register 0 */
  0x10, 0x00, /* R16 palette address 0 */
  0x11, 0x00, /* R17 indirect register pointer */
  0x12, 0x00, /* R18 display adjust */
  0x13, 0x00, /* R19 interrupt line */
  0x17, 0x00  /* R23 display offset */
};

/* EVPC default 16-color palette, 2 bytes per entry. */
static const unsigned char evpc_palette[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x33, 0x07,
  0x07, 0x00, 0x27, 0x03, 0x50, 0x00, 0x07, 0x06,
  0x70, 0x00, 0x73, 0x03, 0x60, 0x06, 0x64, 0x06,
  0x30, 0x04, 0x65, 0x03, 0x55, 0x05, 0x77, 0x07
};

static int is_9938_family() {
  unsigned char original;
  unsigned char observed;
  int detected = 0;

  // Save the current page-0 byte so the test can restore it.
  VDP_SET_REGISTER(0x0e, 0);
  VDP_SET_ADDRESS(0);
  original = VDPRD;

  // Write a sentinel into page 0, then read the same offset from page 2.
  // If R14 selects VRAM pages, page 2 does not contain the sentinel.
  VDP_SET_REGISTER(0x0e, 0);
  VDP_SET_ADDRESS_WRITE(0);
  VDPWD = 0xAA;

  VDP_SET_REGISTER(0x0e, 2);
  VDP_SET_ADDRESS(0);
  observed = VDPRD;

  if (observed != 0xAA) {
    detected = 1;
  } else {
    // The alternate page happened to contain >AA. Try a second sentinel so
    // a single coincidental match cannot be mistaken for no paging.
    VDP_SET_REGISTER(0x0e, 0);
    VDP_SET_ADDRESS_WRITE(0);
    VDPWD = 0x55;

    VDP_SET_REGISTER(0x0e, 2);
    VDP_SET_ADDRESS(0);
    observed = VDPRD;

    if (observed != 0x55) {
      detected = 1;
    }
  }

  // Restore the byte we changed and leave R14 on page 0. On a 9918 the R14
  // writes alias into R6; this leaves R6 at 0 as the console expects.
  VDP_SET_REGISTER(0x0e, 0);
  VDP_SET_ADDRESS_WRITE(0);
  VDPWD = original;

  return detected;
}

static void init_evpc_registers() {
  int i;
  for (i = 0; i < (int)sizeof(evpc_regs); i += 2) {
    VDP_SET_REGISTER(evpc_regs[i], evpc_regs[i + 1]);
  }
}

static void load_evpc_palette() {
  int i;
  VDP_SET_REGISTER(0x10, 0);
  for (i = 0; i < (int)sizeof(evpc_palette); i++) {
    VDPWP = evpc_palette[i];
  }
  VDP_SET_REGISTER(0x10, 0);
}

int vdp_evpc_detect() {
  return is_9938_family();
}

void vdp_evpc_preinit() {
  init_evpc_registers();
  load_evpc_palette();
  VDP_SET_REGISTER(0x0e, 0);
}
