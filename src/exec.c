#include "banks.h"

#define MYBANK BANK(0)

#include "exec.h"
#include "globals.h"
#include "strutil.h"
#include "parsing.h"


int exec_cmd(char* command) {
  // we can't just use handleCommand without implementing some protections for the API caller.
  // 0. set a flag, so we know we are coming in from an API
  api_exec = 1;

  // copy the command onto the stack, so that upper memory expansion can be paged out.
  char stack_command[256];
  bk_strncpy(stack_command, command, 255);
  int result = bk_handleCommand(stack_command);

  api_exec = 0;
  return result;
}
