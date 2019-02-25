#include "banks.h"
#define MYBANK BANK_2

#include "b2_commands.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b2_lvl2.h"
#include <conio.h>

void handleProtect() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cputs("error, must specify a file name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_protect(currentDsr->crubase, unit, filename, 1);
  if (err) {
    cputs("cannot protect file ");
    cputs(currentPath);
    cputs(filename);
    cputc('\n');
  }
}
