#include "fctest.h"

/*
 * I5 - system variables.
 *
 * Covers: var_set, var_get, including case-insensitive names, overwrite,
 * removal via empty or NULL value, and the name/value length limits.
 *
 * var_set and var_get uppercase the name in place, so every name passed here
 * is a mutable buffer. String literals would be written to ROM.
 */

#define UNSET ((char*) -1)

static void test_set_get(void) {
    char name[] = "TESTVAR";
    char lower[] = "testvar";
    char* v;

    var_set(name, "hello world");
    v = var_get(name);
    FC_CHECK(v != UNSET);
    if (v != UNSET) {
        FC_CHECK_EQ(str_cmp(v, "hello world"), 0);
    }

    /* names are case-insensitive: set upper, read lower */
    v = var_get(lower);
    FC_CHECK(v != UNSET);
    if (v != UNSET) {
        FC_CHECK_EQ(str_cmp(v, "hello world"), 0);
    }

    /* overwrite */
    var_set(name, "second");
    v = var_get(name);
    FC_CHECK_EQ(str_cmp(v, "second"), 0);

    /* overwrite through a lowercase name */
    var_set(lower, "third");
    v = var_get(name);
    FC_CHECK_EQ(str_cmp(v, "third"), 0);
}

static void test_remove(void) {
    char name[] = "GONEVAR";
    char* v;

    v = var_get(name);
    FC_CHECK(v == UNSET);

    var_set(name, "temporary");
    v = var_get(name);
    FC_CHECK(v != UNSET);

    /* an empty value removes the variable */
    var_set(name, "");
    v = var_get(name);
    FC_CHECK(v == UNSET);

    /* a NULL value also removes the variable */
    var_set(name, "temporary2");
    var_set(name, 0);
    v = var_get(name);
    FC_CHECK(v == UNSET);
}

static void test_unknown(void) {
    char name[] = "ZZAPITEST9";

    /* guarantee it is absent before checking the not-found result */
    var_set(name, "");
    FC_CHECK(var_get(name) == UNSET);
}

static void test_limits(void) {
    char name[] = "LIMVAR";
    char ok[82];
    char toolong[83];
    char longname[] = "ABCDEFGHIJKL";  /* 12 chars, limit is 11 */
    char* v;
    int i;

    for (i = 0; i < 81; i++) {
        ok[i] = 'a';
    }
    ok[81] = 0;

    var_set(name, ok);
    v = var_get(name);
    FC_CHECK(v != UNSET);
    if (v != UNSET) {
        FC_CHECK_EQ(str_len(v), 81);
    }

    /* 82 characters exceeds the value limit; the old value must remain */
    for (i = 0; i < 82; i++) {
        toolong[i] = 'b';
    }
    toolong[82] = 0;

    var_set(name, toolong);
    v = var_get(name);
    FC_CHECK(v != UNSET);
    if (v != UNSET) {
        FC_CHECK_EQ(str_len(v), 81);
        FC_CHECK_EQ(v[0], 'a');
    }

    /* a name past the limit is rejected and not stored */
    var_set(longname, "x");
    FC_CHECK(var_get(longname) == UNSET);

    /* clean up the large value */
    var_set(name, "");
}

int main(char* args) {
    (void)args;

    term_puts("VARTEST test\n");

    test_set_get();
    test_remove();
    test_unknown();
    test_limits();

    return fc_summary();
}
