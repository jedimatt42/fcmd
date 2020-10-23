#include "banks.h"

#define MYBANK BANK(0)

#include "b0_exec.h"

void exec(const char* command) {
  // we can't just use handleCommand without implementing some protections for the API caller.
  // 0. set a flag, so we know we are coming in from an API - executable loader may need to fail
  // If SAMS:
  // 1. command should be copied onto the stack if it is not already, so that upper memory expansion can be paged out.
  // 2. page out existing executable ( probably belongs over in the executable loader )
  // 3. handleCommand(command);
  // 4. restore previous sams state so we can return to the api caller
  // If NOT SAMS:
  // 1. handleCommand(command) - but expect it to fail / error check / if our flag from step 0 is set and the command includes running an executable.
}