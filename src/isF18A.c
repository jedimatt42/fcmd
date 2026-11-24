#include "banks.h"
#define MYBANK BANK(5)

#include "setupScreen.h"
#include "vdp_internal.h"
#include <vdp.h>

int isF18A() {
  // Ensure a known locked state before unlocking. If the F18A is already
  // unlocked (for example after a soft reset out of an 80x30 session), the
  // unlock sequence does not behave deterministically and the GPU test below
  // can fail. lock_f18a() writes VR50 bit 7, which reboots the F18A and
  // re-locks it when it is unlocked, and is a no-op when already locked.
  bk_lock_f18a();
  bk_unlock_f18a();
  char testcode[6] = { 0x04, 0xE0, 0x3F, 0x00, 0x03, 0x40 };
  vdpmemcpy(0x3F00, testcode, 6);
  {
    VDP_SET_REGISTER(0x36, 0x3F);
    VDP_SET_REGISTER(0x37, 0x00);
  }

  int frames = 6;
  while(frames--) {
    VDP_SET_ADDRESS(0x3F00);
    int res = VDPRD;
    if (!res) {
      return 1;
    }
  }

  return 0;
}
