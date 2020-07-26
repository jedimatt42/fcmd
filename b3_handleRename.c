#include "banks.h"
#define MYBANK BANK(3)

#include "commands.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b2_lvl2.h"
#include <string.h>


void handleRename() {
  char* filename = bk_strtok(0, ' ');
  if (filename == 0) {
    tputs_rom("error, must specify source file name\n");
    return;
  }
  char* newname = bk_strtok(0, ' ');
  if (newname == 0) {
    tputs_rom("error, must specify new file name\n");
    return;
  }

  unsigned int unit = bk_path2unitmask(currentPath);

  char path[256];
  strcpy(path, currentPath);
  bk_strcat(path, filename);

  unsigned int stat = bk_existsDir(currentDsr, path);
  unsigned int ferr = 0x00FF;

  bk_lvl2_setdir(currentDsr->crubase, unit, currentPath);

  if (stat == 0) {
    ferr = bk_lvl2_rendir(currentDsr->crubase, unit, filename, newname);
  } else {
    ferr = bk_lvl2_rename(currentDsr->crubase, unit, filename, newname);
  }

  if (ferr) {
    tputs_rom("cannot rename file ");
    bk_tputs_ram(currentPath);
    bk_tputs_ram(filename);
    bk_tputc('\n');
  }
}
