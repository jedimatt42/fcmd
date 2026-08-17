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

int runExecutable(const char * ext) {
    int err = 0;
    int arg1 = (int)(ext + bk_strlen(ext) + 1);

    __asm__(
        "li r1,fc_exit_return\n\t"
        "bl @fc_exit_prepare\n\t"
        "mov @>A006,r0\n\t"
        "mov %1,r1\n\t"
        "bl *r0\n\t"
        "fc_exit_return:\n\t"
        "mov r1,%0\n\t"
        : "=r"(err)  /* output list */
        : "r"(arg1)  /* input list */
        : "r0", "r1" /* register clobber list */
    );

    procInfoPtr->exit_active = 0;
    return err;
}
