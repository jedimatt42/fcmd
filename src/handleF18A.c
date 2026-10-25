#include "banks.h"
#define MYBANK BANK(0)

#include "commands.h"
#include "detect_vdp.h"
#include "globals.h"
#include "palette.h"
#include "setupScreen.h"
#include "terminal.h"

/*
 * Switch to the F18A personality and initialize its 80x30 color text mode.
 *
 * Detection deliberately does not probe for an F18A when a V9938/V9958 is
 * present: on the CaDD hybrid that probe unlocks the F18A and permanently
 * switches the card away from its V9938 personality until a power cycle.
 * This command performs that switch only when the user explicitly asks for it.
 */
int handleF18A() {
  if (vdp_type != VDP_F18A) {
    if (!bk_isF18A()) {
      // The probe may have disturbed VDP registers, so restore the current
      // screen mode before reporting the failure.
      int width = (displayWidth == 40 || vdp_type == VDP_9918) ? 40 : 80;
      bk_setupScreen(width);
      tputs_rom("no F18A detected\n");
      return 1;
    }
    vdp_type = VDP_F18A;
  }

  bk_setupScreen(80);
  bk_set_palette(0, 0);
  return 0;
}
