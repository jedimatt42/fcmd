#include "banks.h"
#define MYBANK BANK(3)

#include "commands.h"
#include "terminal.h"
#include <string.h>
#include "strutil.h"
#include "globals.h"
#include "parsing.h"
#include "dsrutil.h"
#include "lvl2.h"

int handleMkdir() {
  struct DeviceServiceRoutine *dsr;
  char path[256];
  bk_parsePathParam(0, &dsr, path, PR_REQUIRED);
  if (dsr == 0)
  {
    tputs_rom("no directory path name specified\n");
  return 0;
  }

  unsigned int iocode = bk_path2iocode(path);

  int parent_idx = bk_lindexof(path, '.', bk_strlen(path)-1);
  char dirname[11];
  bk_strncpy(dirname, path+parent_idx + 1, 10);
  path[parent_idx+1] = 0x00;

  bk_lvl2_setdir(dsr->crubase, iocode, path);

  unsigned int err = bk_lvl2_mkdir(dsr->crubase, iocode, dirname);
  if (err) {
    tputs_rom("failed to create directory ");
    bk_tputs_ram(path);
    bk_tputs_ram(dirname);
    bk_tputc('\n');
  }
  return 0;
}
