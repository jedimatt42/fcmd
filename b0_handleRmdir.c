#include "banks.h"
#define MYBANK BANK_0

#include "commands.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b2_lvl2.h"
#include <conio.h>
#include <string.h>

void handleRmdir() {
  char* dirname = strtok(0, " ");
  if (dirname == 0) {
    cputs("error, must specify a directory name\n");
    return;
  }

  unsigned char unit = bk_path2unitmask(currentPath);

  bk_lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned int err = bk_lvl2_rmdir(currentDsr->crubase, unit, dirname);
  if (err) {
    cputs("cannot remove directory ");
    cputs(currentPath);
    cputs(dirname);
    cputc('\n');
  }
}