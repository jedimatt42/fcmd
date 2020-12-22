#include "banks.h"
#define MYBANK BANK(5)

#include "b5_clock.h"
#include "b8_terminal.h"
#include "b1_strutil.h"
#include "b10_sys_info.h"
#include "b10_detect_vdp.h"
#include "b0_globals.h"
#include "b0_sams.h"

void handleSysInfo() {
  struct DisplayInformation di;
  bk_display_info(&di);

  tputs_rom("Display: ");
  if (di.vdp_type == VDP_9918) {
    if (di.isPal) {
      tputs_rom("TMS9929A ");
    } else {
      tputs_rom("TMS9918A ");
    }
  } else if (di.vdp_type == VDP_9938) {
    tputs_rom("V9938 ");
  } else if (di.vdp_type == VDP_9958) {
    tputs_rom("V9958 ");
  } else if (di.vdp_type == VDP_F18A) {
    tputs_rom("F18A ");
  } else {
    tputs_rom("unknown ");
  }

  if (di.isPal) {
    tputs_rom("PAL\n");
  } else {
    tputs_rom("NTSC\n");
  }

  tputs_rom("Memory:  ");
  if (sams_total_pages) {
    unsigned int ram = sams_total_pages * 4;
    bk_tputs_ram(bk_uint2str(ram));
  } else {
    tputs_rom("32");
  }
  tputs_rom("K\n");

  // todo: add a few other options
  // add command line options, default to all, that control what line items to include
}