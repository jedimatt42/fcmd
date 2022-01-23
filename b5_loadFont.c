#include "banks.h"
#define MYBANK BANK(5)

#include <string.h>
#include "b0_globals.h"
#include "b5_loadFont.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b4_variables.h"
#include "b10_parsing.h"
#include <vdp.h>

int load_font() {
  char default_font[256];
  char* env_font = bk_vars_get(str2ram("FONT"));
  if (env_font) {
    bk_strcpy(default_font, env_font);
  } else {
    bk_strcpy(default_font, str2ram("TIPI.FC.FCFONT"));
  }
  struct DeviceServiceRoutine* dsr = 0;
  char filepath[256];
  bk_parsePathParam(default_font, &dsr, filepath, PR_REQUIRED);
  struct PAB pab;
  return bk_dsr_prg_load(dsr, &pab, filepath, gPattern, 8 * 256);
}

