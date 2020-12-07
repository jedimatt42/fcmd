#include "banks.h"

#define MYBANK BANK(1)

#include <conio.h>
#include "commands.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b8_getstr.h"
#include "b4_variables.h"
#include <string.h>

void handleReadkey() {
  char* name = bk_strtok(0, ' ');
  if (!name) {
    tputs_rom("Error, variable name must be given\n");
    return;
  }

  char value[2];
  value[1] = 0;
  value[0] = bk_cgetc(CUR_OVERWRITE);
  bk_vars_set(name, value);
}
