#include "banks.h"
#define MYBANK BANK_2

#include "commands.h"
#include "b1cp_terminal.h"
#include <string.h>
#include "b1cp_strutil.h"
#include "b0_globals.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"

void handleMkdir() {
  char* dirname = strtok(0, " ");
  if (dirname == 0) {
    tputs("error, must specify a directory name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned int err = lvl2_mkdir(currentDsr->crubase, unit, dirname);
  if (err) {
    tputs("cannot create directory ");
    tputs(currentPath);
    tputs(dirname);
    tputc('\n');
  }
}
