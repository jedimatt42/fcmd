#include "banks.h"
#define MYBANK BANK(3)

#include "commands.h"
#include "b0_parsing.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b8_terminal.h"
#include "b2_lvl2.h"
#include <string.h>

void handleRmdir() {
  struct DeviceServiceRoutine *dsr;
  char path[256];
  bk_parsePathParam(&dsr, path, PR_REQUIRED);
  if (dsr == 0)
  {
    tputs_rom("no directory path name specified\n");
    return;
  }

  unsigned int unit = bk_path2unitmask(path);

  int parent_idx = lindexof(path, '.', strlen(path) - 1);
  char dirname[11];
  strncpy(dirname, path + parent_idx + 1, 10);
  path[parent_idx + 1] = 0x00;

  bk_lvl2_setdir(dsr->crubase, unit, path);

  unsigned int err = bk_lvl2_rmdir(dsr->crubase, unit, dirname);
  if (err) {
    tputs_rom("cannot remove directory ");
    bk_tputs_ram(path);
    bk_tputs_ram(dirname);
    bk_tputc('\n');
  }
}
