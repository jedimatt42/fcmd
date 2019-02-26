#include "banks.h"
#define MYBANK BANK_2

#include "b2_commands.h"

/*
void handleRmdir() {
  char* dirname = strtok(0, " ");
  if (dirname == 0) {
    cprintf("error, must specify a directory name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_rmdir(currentDsr->crubase, unit, dirname);
  if (err) {
    cprintf("cannot remove directory %s%s\n", currentPath, dirname);
  }
}
*/

