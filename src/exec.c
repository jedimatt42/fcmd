#include "banks.h"

#define MYBANK BANK(0)

#include "exec.h"
#include "globals.h"
#include "strutil.h"
#include "parsing.h"
#include "sams.h"


int exec_cmd(char* command) {
  // we can't just use handleCommand without implementing some protections for the API caller.
  // 0. set a flag, so we know we are coming in from an API - executable loader may need to fail
  api_exec = 1;

  int result = 0;
  // If SAMS:
  // 1. command should be copied onto the stack if it is not already, so that upper memory expansion can be paged out.
  if (sams_next_page < sams_total_pages) {
    char stack_command[256];
    bk_strncpy(stack_command, command, 255);
    result = bk_handleCommand(stack_command);
  } else {
    result = bk_handleCommand(command);
  }
  api_exec = 0;
  return result;
}
