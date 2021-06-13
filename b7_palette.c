#include "banks.h"
#define MYBANK BANK(7)

#include "b7_palette.h"
#include <vdp.h>

const char DEFAULT_PALETTE[32] = {
  0x00, 0x00, 0x00, 0x00, 0x02, 0xc3, 0x05, 0xd6,
  0x05, 0x4f, 0x07, 0x6f, 0x0d, 0x54, 0x04, 0xef,
  0x0f, 0x54, 0x0f, 0x76, 0x0d, 0xc3, 0x0e, 0xd6,
  0x02, 0xb2, 0x0c, 0x5c, 0x0c, 0xcc, 0x0f, 0xff,
};

void set_palette(int palset, const char* palette) {
  if (palette == 0) {
      palette = DEFAULT_PALETTE;
  }

  unsigned char reg_value = 0xC0 + ((palset & 0x03) << 4);

  // enable write to f18A palette
  // assuming in an F18A unlocked state
  VDP_SET_REGISTER(47, reg_value);

  for (int i = 0; i < 32;i++) {
      VDPWD = palette[i];
  }

  // disable write to palette
  VDP_SET_REGISTER(47, 0x00);
}
