#include "banks.h"

#define MYBANK BANK(4)

#include "commands.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "string.h"
#include "b4_preprocess.h"


void handleEcho() {
  char* peek = strtokpeek(0, " ");
  int newline = (0 != strcmpi("/n", peek));

  if (!newline) {
    strtok(0, " "); // consume the optional /n
  }

  char* tok = strtok(0, 0);
  if (tok) {
    tputs_ram(tok);
  }

  if (newline) {
    tputc('\n');
  }
}