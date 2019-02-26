#include "banks.h"
#define MYBANK BANK_2

#include "b2_commands.h"
#include <conio.h>
#include <string.h>
#include "b1cp_strutil.h"
#include "b0_globals.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"

void handleMkdir() {
  char* dirname = strtok(0, " ");
  if (dirname == 0) {
    cputs("error, must specify a directory name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned int err = lvl2_mkdir(currentDsr->crubase, unit, dirname);
  if (err) {
    cputs("cannot create directory ");
    cputs(currentPath);
    cputs(dirname);
    cputc('\n');
  }
}
