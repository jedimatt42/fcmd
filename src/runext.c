#include "banks.h"

#define MYBANK BANK(0)

#include "runext.h"
#include "procinfo.h"
#include "strutil.h"

/*
  Using code in bank 0, ensures that the API table in bank 0 is available
  to the executable program as it executes and that we return from the
  program in the same bank.
*/

/* Resident process context. The running executable's stack page is mapped at
 * >3000, so it cannot hold its own ProcInfo. */
struct ProcInfo activeProcInfo;

/*
 * Enter an executable on its own SAMS stack page.
 *   r1 = SAMS page to map at >3000
 *   r2 = executable main address (from >A006)
 *   r3 = argument string
 * Returns the status in r1. Implemented in runexec.asm.
 */
extern int run_external(int stack_page, int main_addr, int arg);

int runExecutable(const char * ext) {
    int arg1 = (int)(ext + bk_strlen(ext) + 1);
    int stack_page = procInfoPtr->stack_page;
    int main_addr = *(volatile int*)0xA006;

    int err = run_external(stack_page, main_addr, arg1);

    procInfoPtr->exit_active = 0;
    return err;
}
