#include "banks.h"
#define MYBANK BANK(5)

#include "clock.h"
#include "terminal.h"
#include "strutil.h"
#include "sys_info.h"
#include "graphics.h"
#include "detect_vdp.h"
#include "globals.h"
#include "sams.h"
#include "dictionary.h"

int handleSysInfo() {
  int show_mem = 0;
  char* peek = bk_strtokpeek(0, ' ');
  if (peek && 0 == bk_strcmpi(str2ram("/m"), peek)) {
    show_mem = 1;
    bk_strtok(0, ' '); // consume the optional /m
  }

  struct GfxInformation di;
  bk_gfx_get_info(&di);

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

#ifdef __TMS9900_PATCH_MAJOR__
  int gcc_major = __TMS9900_PATCH_MAJOR__;
  int gcc_minor = __TMS9900_PATCH_MINOR__;
  tputs_rom("GCC:     ");
  bk_tputs_ram(bk_uint2str(gcc_major));
  tputs_rom(".");
  bk_tputs_ram(bk_uint2str(gcc_minor));
  tputs_rom("\n");
#endif

  if (show_mem) {
    char* sp;
    __asm__("mov r10,%0" : "=r"(sp));
    tputs_rom("Dict:    ");
    bk_tputs_ram(bk_uint2hex((unsigned int) system_dict.end));
    tputs_rom("  SP: ");
    bk_tputs_ram(bk_uint2hex((unsigned int) sp));
    tputs_rom("\n");
  }

  // todo: add a few other options
  // add command line options, default to all, that control what line items to include
  return 0;
}
