#include "banks.h"

#define MYBANK BANK(10)

#include "detect_vdp.h"
#include "setupScreen.h"
#include <vdp.h>

int vdp_type;

int detect_vdp() {
  unsigned char reg4 = 0;
  VDPWA = 4;
  VDPWA = 0x80 | 15;

  // Status register 4 has its 7 high bits set on a Yamaha VDP. Read it twice:
  // a 9918A/locked F18A ignores the status-register select and returns SR0,
  // whose interrupt/5S/collision flags clear after the first read.
  reg4 = VDPST & 0xFE;
  if (reg4 == 0xFE && (VDPST & 0xFE) != 0xFE) {
    reg4 = 0;
  }

  VDPWA = 0;
  VDPWA = 0x80 | 15;

  #ifdef _CLASSIC_99

  if (reg4 != 0xFE) {
    if (bk_isF18A()) {
      return VDP_F18A;
    }
  }

  return VDP_9938;

  #else

  if (reg4 != 0xFE) {
    // Only probe for an F18A when no Yamaha VDP answered. On a V9938/V9958
    // the probe would unlock an F18A and, on the CaDD hybrid, permanently
    // switch the card away from its V9938 personality.
    if (bk_isF18A()) {
      return VDP_F18A;
    }
    return VDP_9918;
  } else {
    unsigned char reg1 = 0;
    VDPWA = 1;
    VDPWA = 0x80 | 15;

    reg1 = VDPST;

    VDPWA = 0;
    VDPWA = 0x80 | 15;

    if (reg1 & 0x04) {
        return VDP_9958;
    }

    return VDP_9938;

  }

  #endif
}