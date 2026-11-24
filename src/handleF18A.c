#include "banks.h"
#define MYBANK BANK(1)

#include "commands.h"
#include "detect_vdp.h"
#include "globals.h"
#include "palette.h"
#include "setupScreen.h"
#include "strutil.h"
#include "terminal.h"
#include "vdp9918.h"
#include "vdp_internal.h"

/*
 * Explicitly switch to the F18A personality and select its 80x24 attribute
 * text mode. Use 'height 30' afterwards for T80x30 when desired.
 *
 * 'f18a lock' instead leaves the F18A locked, initializes all standard 9918
 * registers, and selects 40-column text mode.
 *
 * ForceCommand never probes for an F18A automatically: on the CaDD hybrid
 * that probe unlocks the F18A and permanently switches the card away from
 * its V9938 personality until a power cycle. Only this command performs it.
 */
int handleF18A() {
  char* opt = bk_strtok(0, ' ');
  int lock = 0;

  if (opt) {
    if (0 == bk_strcmpi(str2ram("lock"), opt)) {
      lock = 1;
    } else {
      tputs_rom("unknown f18a option\n");
      return 1;
    }
  }

  if (vdp_type != VDP_F18A) {
    if (!bk_isF18A()) {
      // The probe may have disturbed VDP registers, so restore the current
      // screen mode before reporting the failure.
      bk_setupScreenEx(displayWidth, displayHeight);
      tputs_rom("no F18A detected\n");
      return 1;
    }
    vdp_type = VDP_F18A;
  }

  if (lock) {
    // Write the palette while the F18A is unlocked; locking leaves the
    // palette contents untouched.
    bk_unlock_f18a();
    bk_set_palette(0, 0);
    bk_lock_f18a();
    bk_setupScreenEx(40, 24);
    bk_vdp9918_init_text40();
    return 0;
  }

  bk_setupScreenEx(80, 24);
  bk_set_palette(0, 0);
  return 0;
}
