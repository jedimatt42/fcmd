#include "banks.h"
#define MYBANK BANK_0

#include "commands.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b2_lvl2.h"
#include <string.h>
#include <conio.h>


void handleRename() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cputs("error, must specify source file name\n");
    return;
  }
  char* newname = strtok(0, " ");
  if (newname == 0) {
    cputs("error, must specify new file name\n");
    return;
  }

  unsigned char unit = bk_path2unitmask(currentPath);

  char path[256];
  strcpy(path, currentPath);
  strcat(path, ".");
  strcat(path, filename);

  unsigned int stat = bk_existsDir(currentDsr, path);

  bk_lvl2_setdir(currentDsr->crubase, unit, currentPath);
  unsigned int err = 0x00ff;
  if (stat == 0) {
    err = bk_lvl2_rendir(currentDsr->crubase, unit, filename, newname);
  } else {
    err = bk_lvl2_rename(currentDsr->crubase, unit, filename, newname);
  }

  if (err) {
    cputs("cannot rename file ");
    cputs(currentPath);
    cputs(filename);
    cputc('\n');
  }
}
