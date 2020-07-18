#include "banks.h"
#define MYBANK BANK(4)

#include "commands.h"
#include "b8_terminal.h"
#include <string.h>
#include "b1_strutil.h"
#include "b0_globals.h"
#include "b0_parsing.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"

void handleMkdir() {
  struct DeviceServiceRoutine *dsr;
  char path[256];
  bk_parsePathParam(&dsr, path, PR_REQUIRED);
  if (dsr == 0)
  {
    tputs_rom("no directory path name specified\n");
    return;
  }

  unsigned int unit = bk_path2unitmask(path);

  int parent_idx = lindexof(path, '.', strlen(path)-1);
  char dirname[11];
  strncpy(dirname, path+parent_idx + 1, 10);
  path[parent_idx+1] = 0x00;

  bk_lvl2_setdir(dsr->crubase, unit, path);

  unsigned int err = bk_lvl2_mkdir(dsr->crubase, unit, dirname);
  if (err) {
    tputs_rom("failed to create directory ");
    bk_tputs_ram(path);
    bk_tputs_ram(dirname);
    bk_tputc('\n');
  }
}
