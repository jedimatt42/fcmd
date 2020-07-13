#include "banks.h"

#define MYBANK BANK(4)

#include "commands.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b4_variables.h"

void handleEnv() {
  printVars();
}