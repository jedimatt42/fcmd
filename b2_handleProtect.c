#include "banks.h"
#define MYBANK BANK_2

#include "commands.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b2_lvl2.h"
#include "b1cp_terminal.h"

static void doProtect(char mode);

void handleProtect() {
  doProtect(1);
}

void handleUnprotect() {
  doProtect(0);
}

static void doProtect(char mode) {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    tputs("error, must specify a file name\n");
    return;
  }

  unsigned int unit = path2unitmask(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned int err = lvl2_protect(currentDsr->crubase, unit, filename, mode);
  if (err) {
    tputs("cannot protect file ");
    tputs(currentPath);
    tputs(filename);
    tputc('\n');
  }
}
