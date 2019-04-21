#include "banks.h"

#define MYBANK BANK_4

#include <conio.h>
#include "commands.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b4_variables.h"

void handleReadkey() {
  char* name = strtok(0, " ");
  if (!name) {
    tputs("Error, variable name must be given\n");
    return;
  }

  char value[2];
  value[1] = 0;
  value[0] = cgetc();
  vars_set(name, value);
}