#include "banks.h"

#define MYBANK BANK(10)

#include "detect_vdp.h"
#include "setupScreen.h"
#include "vdp_evpc.h"
#include "vdp_internal.h"
#include <vdp.h>

int vdp_type;

/*
 * Reset values for the V9938/V9958 control registers, R#0 through R#46.
 *
 * R#8 selects 64K VRAM. The remaining table/address and command registers are
 * cleared so a board that did not reset cleanly cannot leave a stale base
 * address or pending command behind. R#20-R#22 are the composite color burst
 * registers; zeroing those erases the burst, so keep their documented reset
 * values.
 */
static const unsigned char vdp9938_regs[0x2F] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* R0-R7 */
  0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* R8-R15 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3B, 0x05, 0x00, /* R16-R23 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* R24-R31 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* R32-R39 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        /* R40-R46 */
};

/* V9938/V9958 palette values programmed on reset (see the V9938 manual). */
static const unsigned char vdp9938_palette[32] = {
  0x00, 0x00, 0x00, 0x00, 0x11, 0x06, 0x33, 0x07,
  0x17, 0x01, 0x27, 0x03, 0x51, 0x01, 0x27, 0x06,
  0x71, 0x01, 0x73, 0x03, 0x61, 0x06, 0x64, 0x06,
  0x11, 0x04, 0x65, 0x02, 0x55, 0x05, 0x77, 0x07
};

/*
 * If an executable left a V9938/V9958 command running, let it finish before
 * reprogramming registers or clearing VRAM. The timeout keeps a wedged
 * command engine from preventing ForceCommand from starting.
 */
static void wait_vdp_idle() {
  unsigned int timeout = 0xFFFF;
  VDP_SET_REGISTER(0x0f, 2);
  while ((VDPST & 0x01) && --timeout) {
  }
  VDP_SET_REGISTER(0x0f, 0);
}

/*
 * Clear 16K VRAM banks. Each bank is selected through R#14 and addressed from
 * zero so the result does not depend on the auto-increment behaviour of the
 * VDP's current display mode.
 */
static void clear_vdp_banks(int banks) {
  int bank;
  int i;
  for (bank = 0; bank < banks; bank++) {
    if (banks > 1) {
      VDP_SET_REGISTER(0x0e, bank);
    }
    VDP_SET_ADDRESS_WRITE(0);
    for (i = 0; i < 0x4000; i++) {
      VDPWD = 0;
    }
  }
  if (banks > 1) {
    VDP_SET_REGISTER(0x0e, 0);
  }
}

int detect_vdp() {
  unsigned char reg4 = 0;
  unsigned char reg1 = 0;

  // First use the EVPC VRAM-page method. Unlike the F18A probe this cannot
  // unlock an F18A and switch a CaDD hybrid away from its V9938 personality.
  if (bk_vdp_evpc_detect()) {
    bk_vdp_evpc_preinit();

    VDP_SET_REGISTER(0x0f, 1);
    reg1 = VDPST;
    VDP_SET_REGISTER(0x0f, 0);

    return (reg1 & 0x04) ? VDP_9958 : VDP_9938;
  }

  // Fallback for a Yamaha VDP whose page-0/page-2 bytes happened to match:
  // status register 4 has its 7 high bits set on a V9938/V9958. Read it
  // twice; a 9918A/locked F18A ignores the status-register select and returns
  // SR0, whose interrupt/5S/collision flags clear after the first read.
  VDP_SET_REGISTER(0x0f, 4);
  reg4 = VDPST & 0xFE;
  if (reg4 == 0xFE && (VDPST & 0xFE) != 0xFE) {
    reg4 = 0;
  }
  VDP_SET_REGISTER(0x0f, 0);

  if (reg4 == 0xFE) {
    VDP_SET_REGISTER(0x0f, 1);
    reg1 = VDPST;
    VDP_SET_REGISTER(0x0f, 0);

    bk_vdp_evpc_preinit();
    return (reg1 & 0x04) ? VDP_9958 : VDP_9938;
  }

  #ifdef _CLASSIC_99

  if (bk_isF18A()) {
    return VDP_F18A;
  }
  bk_vdp_evpc_preinit();
  return VDP_9938;

  #else

  // Only probe for an F18A when no V9938/V9958 answered. On a V9938/V9958
  // the probe would unlock an F18A and, on the CaDD hybrid, permanently
  // switch the card away from its V9938 personality.
  if (bk_isF18A()) {
    return VDP_F18A;
  }
  return VDP_9918;

  #endif
}

void vdp_full_init() {
  int i;

  if (vdp_type == VDP_9938 || vdp_type == VDP_9958) {
    wait_vdp_idle();
    for (i = 0; i < 0x2F; i++) {
      VDP_SET_REGISTER(i, vdp9938_regs[i]);
    }
    clear_vdp_banks(4);

    // Palette writes use R#16 as a pointer and then the palette data port.
    VDP_SET_REGISTER(0x10, 0);
    for (i = 0; i < 32; i++) {
      VDPWP = vdp9938_palette[i];
    }
    VDP_SET_REGISTER(0x10, 0);

    // Leave the access, status, and command registers in a known state.
    VDP_SET_REGISTER(0x0e, 0);
    VDP_SET_REGISTER(0x0f, 0);
    return;
  }

  // A locked F18A only exposes the standard registers; lock it before the
  // standard register writes so an unlocked GPU cannot run during the clear.
  if (vdp_type == VDP_F18A) {
    bk_lock_f18a();
  }
  for (i = 0; i < 8; i++) {
    VDP_SET_REGISTER(i, 0);
  }
  clear_vdp_banks(1);
}
