#include "banks.h"

#define MYBANK BANK(4)

#include "commands.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b4_variables.h"

void handleEnv() {
  printVars();
}