#include "banks.h"
#define MYBANK BANK_3

#include "b0_globals.h"
#include "commands.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b2_dsrutil.h"
#include <string.h>

void handleDelete() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    tputs_rom("error, must specify a file name\n");
    return;
  }

  char buffer[256];
  strcpy(buffer, currentPath);
  strcat(buffer, filename);
  
  unsigned int exists = bk_existsFile(currentDsr, buffer);
  if (!exists) {
    tputs_rom("file not found ");
    tputs_ram(buffer);
    tputc('\n');
    return;
  }

  struct PAB pab;
  bk_initPab(&pab);
  pab.pName = buffer;

  unsigned int err = bk_dsr_delete(currentDsr, &pab);
  if (err) {
    tputs_rom("cannot delete file ");
    tputs_ram(currentPath);
    tputs_ram(filename);
    tputc('\n');
  }
}
