#include "banks.h"
#define MYBANK BANK(7)

#include "commands.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b10_detect_vdp.h"
#include "b7_palette.h"
#include <string.h>


void handlePalette() {
  if (vdp_type != VDP_F18A) {
      tputs_rom("only supported on F18A VDP\n");
      return;
  }

  char* tok = bk_strtokpeek(0, ' ');
  if (!bk_strcmpi(str2ram("/r"), tok)) {
      bk_strtok(0, ' '); // consume optional token
      set_palette(0, 0);
      return;
  }

  // otherwise, parse integers from command line from hex
  unsigned int user_pal[16];
  for(int i=0; i<16; i++) {
      user_pal[i] = 0;
  }

  int idx = 0;
  tok = bk_strtok(0, ' ');
  while(tok && idx < 16) {
      user_pal[idx++] = bk_htoi(tok);
      tok = bk_strtok(0, ' ');
  }
  if (idx < 16) {
      tputs_rom("error: 16 palette entries required\n");
      return;
  }
  set_palette(0, (char*) user_pal);
}
