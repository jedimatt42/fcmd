#include "banks.h"
#define MYBANK BANK_3

#include "commands.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b2_lvl2.h"
#include <string.h>

void handleRmdir() {
  char* dirname = strtok(0, " ");
  if (dirname == 0) {
    tputs_rom("error, must specify a directory name\n");
    return;
  }

  unsigned int unit = bk_path2unitmask(currentPath);

  bk_lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned int err = bk_lvl2_rmdir(currentDsr->crubase, unit, dirname);
  if (err) {
    tputs_rom("cannot remove directory ");
    tputs_ram(currentPath);
    tputs_ram(dirname);
    tputc('\n');
  }
}
