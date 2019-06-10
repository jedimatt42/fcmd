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
    tputs("error, must specify a file name\n");
    return;
  }

  char buffer[256];
  strcpy(buffer, currentPath);
  strcat(buffer, filename);
  
  unsigned int exists = bk_existsFile(currentDsr, buffer);
  if (!exists) {
    tputs("file not found ");
    tputs(buffer);
    tputc('\n');
    return;
  }

  struct PAB pab;
  bk_initPab(&pab);
  pab.pName = buffer;

  unsigned int err = bk_dsr_delete(currentDsr, &pab);
  if (err) {
    tputs("cannot delete file ");
    tputs(currentPath);
    tputs(filename);
    tputc('\n');
  }
}
