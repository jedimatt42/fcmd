#include "banks.h"
#define MYBANK BANK_2

#include "b2_commands.h"



/*
void handleProtect() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify a file name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_protect(currentDsr->crubase, unit, filename, 1);
  if (err) {
    cprintf("cannot protect file %s%s\n", currentPath, filename);
  }
}
*/

/*
void handleUnprotect() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify a file name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_protect(currentDsr->crubase, unit, filename, 0);
  if (err) {
    cprintf("cannot unprotect file %s%s\n", currentPath, filename);
  }
}
*/

/*
void handleRename() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify source file name\n");
    return;
  }
  char* newname = strtok(0, " ");
  if (newname == 0) {
    cprintf("error, must specify new file name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  char path[256];
  strcpy(path, currentPath);
  strcat(path, ".");
  strcat(path, filename);

  unsigned char stat = existsDir(currentDsr, path);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);
  unsigned char err = 0xff;
  if (stat == 0) {
    err = lvl2_rendir(currentDsr->crubase, unit, filename, newname);
  } else {
    err = lvl2_rename(currentDsr->crubase, unit, filename, newname);
  }

  if (err) {
    cprintf("cannot rename file %s%s\n", currentPath, filename);
  }
}
*/

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

