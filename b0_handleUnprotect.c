#include "banks.h"
#define MYBANK BANK_0

#include "b2_commands.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b2_lvl2.h"
#include <conio.h>

void handleUnprotect() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cputs("error, must specify a file name\n");
    return;
  }

  unsigned char unit = bk_path2unitmask(currentPath);

  bk_lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned int err = bk_lvl2_protect(currentDsr->crubase, unit, filename, 0);
  if (err) {
    cputs("cannot unprotect file ");
    cputs(currentPath);
    cputs(filename);
    cputc('\n');
  }
}

