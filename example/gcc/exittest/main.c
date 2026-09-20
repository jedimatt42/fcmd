#include "fctest.h"

/*
 * I12 - non-local executable exit.
 *
 * Covers: fc_exit, and the exit(x) macro alias.
 *
 * Because fc_exit does not return, this program behaves differently depending
 * on its argument:
 *   EXITTEST     - exit with status 0 (clean return to the prompt)
 *   EXITTEST 1   - exit with status 1 (ForceCommand prints "error result: 1")
 *
 * Run it both ways to exercise both paths. There is no summary line because
 * the process ends through fc_exit.
 */

static void do_exit(int status) {
    term_puts("EXITTEST: exiting with status ");
    fc_puts_int(status);
    term_putc('\n');

    if (status == 0) {
        FC_OBSERVE("expect a clean return to the ForceCommand prompt, no error line");
    } else {
        FC_OBSERVE("expect ForceCommand to print: error result: N");
    }
    FC_WAIT("press a key to exit");

    if (status == 0) {
        exit(0);        /* the exit(x) macro alias */
    }
    fc_exit(status);    /* fc_exit directly */
}

int main(char* args) {
    int status = args ? str_to_int(args) : 0;
    do_exit(status);
    return 0;   /* not reached */
}
