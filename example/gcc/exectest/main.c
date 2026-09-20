#include "fctest.h"

/*
 * I6 - command execution from an executable.
 *
 * Covers: exec_cmd.
 *
 * exec_cmd preprocesses and tokenizes the command in place, so every command
 * passed here is a mutable buffer. Only built-in commands and an assignment are
 * used, avoiding executable loading while api_exec is set.
 */

static void test_echo(void) {
    char cmd[] = "echo APITEST-ECHO-OK";
    int r;

    term_puts("[OBSERVE] next line should read: APITEST-ECHO-OK\n");
    r = exec_cmd(cmd);
    FC_CHECK_EQ(r, 0);
}

static void test_empty(void) {
    char cmd[] = "";
    FC_CHECK_EQ(exec_cmd(cmd), 0);
}

static void test_assignment(void) {
    char cmd[] = "EXECTEST9=hello";
    char name[] = "EXECTEST9";
    char* v;

    FC_CHECK_EQ(exec_cmd(cmd), 0);

    v = var_get(name);
    FC_CHECK(v != (char*) -1);
    if (v != (char*) -1) {
        FC_CHECK_EQ(str_cmp(v, "hello"), 0);
    }

    /* clean up the variable created by the assignment */
    var_set(name, "");
}

static void test_unknown(void) {
    char cmd[] = "zznotacommand";
    int r;

    term_puts("[OBSERVE] an 'unknown command' error is expected next\n");
    r = exec_cmd(cmd);
    FC_CHECK(r != 0);
}

int main(char* args) {
    (void)args;

    term_puts("EXECTEST test\n");

    test_echo();
    test_empty();
    test_assignment();
    test_unknown();

    return fc_summary();
}
