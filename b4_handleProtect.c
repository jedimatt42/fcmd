#include "banks.h"
#define MYBANK BANK(4)

#include <string.h>
#include "commands.h"
#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include "b2_lvl2.h"
#include "b8_terminal.h"

static void doProtect(char mode);

void handleProtect() {
  doProtect(1);
}

void handleUnprotect() {
  doProtect(0);
}

static void doProtect(char mode) {
  struct DeviceServiceRoutine *dsr;
  char path[256];
  bk_parsePathParam(&dsr, path, PR_REQUIRED);
  if (dsr == 0)
  {
    tputs_rom("no file path name specified\n");
    return;
  }

  unsigned int unit = path2unitmask(path);

  int parent_idx = bk_lindexof(path, '.', strlen(path) - 1);
  char filename[11];
  strncpy(filename, path + parent_idx + 1, 10);
  path[parent_idx + 1] = 0x00;

  lvl2_setdir(dsr->crubase, unit, path);

  unsigned int err = lvl2_protect(dsr->crubase, unit, filename, mode);
  if (err) {
    tputs_rom("failed to modify file ");
    bk_tputs_ram(path);
    bk_tputs_ram(filename);
    bk_tputc('\n');
  }
}
