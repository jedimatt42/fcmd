#include "banks.h"
#define MYBANK BANK_3

#include "commands.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b2_lvl2.h"
#include <string.h>


void handleRename() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    tputs("error, must specify source file name\n");
    return;
  }
  char* newname = strtok(0, " ");
  if (newname == 0) {
    tputs("error, must specify new file name\n");
    return;
  }

  unsigned int unit = bk_path2unitmask(currentPath);

  char path[256];
  strcpy(path, currentPath);
  strcat(path, filename);

  unsigned int stat = bk_existsDir(currentDsr, path);
  unsigned int ferr = 0x00FF;

  bk_lvl2_setdir(currentDsr->crubase, unit, currentPath);

  if (stat == 0) {
    ferr = bk_lvl2_rendir(currentDsr->crubase, unit, filename, newname);
  } else {
    ferr = bk_lvl2_rename(currentDsr->crubase, unit, filename, newname);
  }

  if (ferr) {
    tputs("cannot rename file ");
    tputs(currentPath);
    tputs(filename);
    tputc('\n');
  }
}
