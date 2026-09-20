#include "fctest.h"

/*
 * I0 - harness self test.
 *
 * Proves the fctest.h report protocol works before any API test depends on it.
 * All checks here are expected to pass, so this program returns 0.
 */

int main(char* args) {
    (void)args;

    term_puts("APITEST harness self test\n");

    FC_CHECK(1);
    FC_CHECK(1 == 1);
    FC_CHECK_EQ(2 + 2, 4);
    FC_CHECK_EQ(str_len("abc"), 3);
    FC_SKIP("example skip: no hardware needed for the harness");
    FC_OBSERVE("this line documents how observation prompts appear");
    FC_WAIT("press any key to finish the harness self test");

    return fc_summary();
}
