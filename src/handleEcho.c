#include "banks.h"

#define MYBANK BANK(4)

#include "commands.h"
#include "strutil.h"
#include "terminal.h"
#include "string.h"
#include "preprocess.h"


int handleEcho() {
  char* peek = bk_strtokpeek(0, ' ');
  int newline = (0 != bk_strcmpi(str2ram("/n"), peek));

  if (!newline) {
    bk_strtok(0, ' '); // consume the optional /n
  }

  char* tok = bk_strtok(0, 0);
  if (tok) {
    bk_tputs_ram(tok);
  }

  if (newline) {
    bk_tputc('\n');
  }
  return 0;
}