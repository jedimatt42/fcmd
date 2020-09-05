#include "banks.h"
#define MYBANK BANK(3)

#include "commands.h"
#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b2_dsrutil.h"
#include <string.h>

void handleDelete() {
  struct DeviceServiceRoutine *dsr;
  char path[256];
  bk_parsePathParam(0, &dsr, path, PR_REQUIRED);
  if (dsr == 0)
  {
    tputs_rom("no file path name specified\n");
    return;
  }

  unsigned int exists = bk_existsFile(dsr, path);
  if (!exists) {
    tputs_rom("file not found ");
    bk_tputs_ram(path);
    bk_tputc('\n');
    return;
  }

  struct PAB pab;
  bk_initPab(&pab);
  pab.pName = path;

  unsigned int err = bk_dsr_delete(dsr, &pab);
  if (err) {
    tputs_rom("cannot delete file ");
    bk_tputs_ram(path);
    bk_tputc('\n');
  }
}
