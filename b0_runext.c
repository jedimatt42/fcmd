#include "banks.h"

#define MYBANK BANK(0)

#include "b0_runext.h"
#include "b1_strutil.h"

/*
  Using code in bank 0, ensures that the API table in bank 0 is available
  to the extension program as it executes and that we return from the
  program in the same bank.
*/

int runExtension(const char * ext) {
    int err = 0;
    int arg1 = (int)(ext + bk_strlen(ext) + 1);
    __asm__(
        "mov @>A006,r0\n\t"
        "mov %1,r1\n\t"
        "bl *r0\n\t"
        "mov r1,%0\n\t"
        : "=r"(err)  /* output list */
        : "r"(arg1)  /* input list */
        : "r0", "r1" /* register clobber list */
    );
    return err;
}