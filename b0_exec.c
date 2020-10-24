#include "banks.h"

#define MYBANK BANK(0)

#include "b0_exec.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b0_parsing.h"
#include "b0_sams.h"


int exec(char* command) {
  // we can't just use handleCommand without implementing some protections for the API caller.
  // 0. set a flag, so we know we are coming in from an API - executable loader may need to fail
  api_exec = 1;

  // If SAMS:
  // 1. command should be copied onto the stack if it is not already, so that upper memory expansion can be paged out.
  if (sams_next_page < sams_total_pages) {
    char stack_command[256];
    bk_strncpy(stack_command, command, 255);
    handleCommand(stack_command);
  } else {
    handleCommand(command);
  }
  api_exec = 0;
  return 0; // maybe pass codes back someday
}