#include "banks.h"
#define MYBANK BANK(3)

#include "commands.h"
#include "b10_parsing.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b2_lvl2.h"
#include <string.h>

void handleRmdir() {
  struct DeviceServiceRoutine *dsr;
  char path[256];
  bk_parsePathParam(0, &dsr, path, PR_REQUIRED);
  if (dsr == 0)
  {
    tputs_rom("no directory path name specified\n");
    return;
  }

  unsigned int iocode = bk_path2iocode(path);

  int parent_idx = bk_lindexof(path, '.', bk_strlen(path) - 1);
  char dirname[11];
  bk_strncpy(dirname, path + parent_idx + 1, 10);
  path[parent_idx + 1] = 0x00;

  bk_lvl2_setdir(dsr->crubase, iocode, path);

  unsigned int err = bk_lvl2_rmdir(dsr->crubase, iocode, dirname);
  if (err) {
    tputs_rom("cannot remove directory ");
    bk_tputs_ram(path);
    bk_tputs_ram(dirname);
    bk_tputc('\n');
  }
}
