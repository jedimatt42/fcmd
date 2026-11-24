#include "banks.h"
#define MYBANK BANK(5)

#include "commands.h"
#include "detect_vdp.h"
#include "globals.h"
#include "setupScreen.h"
#include "strutil.h"
#include "terminal.h"

int handleHeight() {
  char* tok = bk_strtok(0, ' ');
  if (!tok) {
    tputs_rom("no supported height specified\n");
    return 0;
  }
  int height = bk_atoi(tok);

  if (height == 24) {
    bk_setupScreenEx(displayWidth, 24);
    return 0;
  }

  if (height == 26 &&
      (vdp_type == VDP_9938 || vdp_type == VDP_9958)) {
    bk_setupScreenEx(displayWidth, 26);
    return 0;
  }

  if (height == 30 && vdp_type == VDP_F18A) {
    bk_setupScreenEx(displayWidth, 30);
    return 0;
  }

  tputs_rom("no supported height specified\n");
  return 0;
}
