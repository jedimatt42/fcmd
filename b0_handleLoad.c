#include "banks.h"

#define MYBANK BANK_0

#include "commands.h"
#include "b0_main.h"
#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include <string.h>
#include <vdp.h>
#include <conio.h>

void handleLoad() {
  struct DeviceServiceRoutine* dsr = 0;

  char path[256];
  bk_parsePathParam(&dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    tputs_rom("no file specified\n");
    return;
  }

  // TODO, don't require exists if a PI.HTTP URL...
  if (path[0] != 'P' || path[1] != 'I') {
    if (!bk_existsFile(dsr, path)) {
      tputs_rom("error, file not found: ");
      tputs_ram(path);
      tputc('\n');
      return;
    }
  }

  // TODO - test that it is ea5-ish

  resetF18A();
  set_graphics(0); // This is mostly write, but we have
  // to override some VDP registers for EA5 compatibility
  // VDP_SET_REGISTER(VDP_REG_MODE0, 0x00);
  VDP_SET_REGISTER(VDP_REG_MODE1, 0xE0);
  // VDP_SET_REGISTER(VDP_REG_SIT, 0x00);
  // VDP_SET_REGISTER(VDP_REG_CT, 0x0E);
  // VDP_SET_REGISTER(VDP_REG_PDT, 0x01);
  // VDP_SET_REGISTER(VDP_REG_SAL, 0x06);
  VDP_SET_REGISTER(VDP_REG_SDT, 0x00);
  VDP_SET_REGISTER(VDP_REG_COL, 0xF3);

  // erase first 4k of vdp
  vdpmemset(0,0,4192);

  // ea clears screen with 0x20 (space characters)
  clrscr();
  // load character set
  charset();
  // TODO load ea copyright and cursor

  // set colors
  vdpmemset(0x0380, 0x13, 32);

  // TODO load ea-utils into >2000

  bk_dsr_ea5load(dsr, path);
}