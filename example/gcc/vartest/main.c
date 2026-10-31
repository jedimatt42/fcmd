#include "fctest.h"

/*
 * I5 - system variables.
 *
 * Covers: var_set, var_get, including case-insensitive names, overwrite,
 * removal via empty or NULL value, the name/value length limits, and the
 * copy-out buffer contract.
 *
 * var_get never returns a pointer into ForceCommand's dictionary. It copies
 * into the caller's buffer and returns 1 when found, 0 when not found.
 */

static void test_set_get(void) {
    char name[] = "TESTVAR";
    char lower[] = "testvar";
    char value[82];
    char value2[82];

    var_set(name, "hello world");
    FC_CHECK(var_get(name, value, sizeof(value)));
    FC_CHECK_EQ(str_cmp(value, "hello world"), 0);

    /* names are case-insensitive: set upper, read lower */
    FC_CHECK(var_get(lower, value, sizeof(value)));
    FC_CHECK_EQ(str_cmp(value, "hello world"), 0);
    /* var_get must not uppercase the caller's name buffer */
    FC_CHECK_EQ(str_cmp(lower, "testvar"), 0);

    /* overwrite */
    var_set(name, "second");
    FC_CHECK(var_get(name, value, sizeof(value)));
    FC_CHECK_EQ(str_cmp(value, "second"), 0);

    /* overwrite through a lowercase name */
    var_set(lower, "third");
    FC_CHECK(var_get(name, value, sizeof(value)));
    FC_CHECK_EQ(str_cmp(value, "third"), 0);

    /* the returned buffer is a copy: changing it must not change the value */
    var_set(name, "copy");
    FC_CHECK(var_get(name, value, sizeof(value)));
    value[0] = 'X';
    FC_CHECK(var_get(name, value2, sizeof(value2)));
    FC_CHECK_EQ(str_cmp(value2, "copy"), 0);
}

static void test_remove(void) {
    char name[] = "GONEVAR";
    char value[82];

    FC_CHECK_EQ(var_get(name, value, sizeof(value)), 0);
    FC_CHECK_EQ(value[0], 0);

    var_set(name, "temporary");
    FC_CHECK(var_get(name, value, sizeof(value)));
    FC_CHECK_EQ(str_cmp(value, "temporary"), 0);

    /* an empty value removes the variable */
    var_set(name, "");
    FC_CHECK_EQ(var_get(name, value, sizeof(value)), 0);

    /* a NULL value also removes the variable */
    var_set(name, "temporary2");
    var_set(name, 0);
    FC_CHECK_EQ(var_get(name, value, sizeof(value)), 0);
}

static void test_unknown(void) {
    char name[] = "ZZAPITEST9";
    char value[82];

    /* guarantee it is absent before checking the not-found result */
    var_set(name, "");
    FC_CHECK_EQ(var_get(name, value, sizeof(value)), 0);
}

static void test_limits(void) {
    char name[] = "LIMVAR";
    char ok[82];
    char toolong[83];
    char longname[] = "ABCDEFGHIJKL";  /* 12 chars, limit is 11 */
    char value[82];
    int i;

    for (i = 0; i < 81; i++) {
        ok[i] = 'a';
    }
    ok[81] = 0;

    var_set(name, ok);
    FC_CHECK(var_get(name, value, sizeof(value)));
    FC_CHECK_EQ(str_len(value), 81);

    /* 82 characters exceeds the value limit; the old value must remain */
    for (i = 0; i < 82; i++) {
        toolong[i] = 'b';
    }
    toolong[82] = 0;

    var_set(name, toolong);
    FC_CHECK(var_get(name, value, sizeof(value)));
    FC_CHECK_EQ(str_len(value), 81);
    FC_CHECK_EQ(value[0], 'a');

    /* a name past the limit is rejected and not stored */
    var_set(longname, "x");
    FC_CHECK_EQ(var_get(longname, value, sizeof(value)), 0);

    /* clean up the large value */
    var_set(name, "");
}

static void test_small_buffer(void) {
    char name[] = "SMALLBUF";
    char value[4];

    var_set(name, "abcdef");
    FC_CHECK(var_get(name, value, sizeof(value)));
    FC_CHECK_EQ(str_cmp(value, "abc"), 0);

    /* a zero or negative limit is rejected and does not write */
    value[0] = 'Z';
    FC_CHECK_EQ(var_get(name, value, 0), 0);
    FC_CHECK_EQ(value[0], 'Z');

    var_set(name, "");
}

static void test_literal_name(void) {
    char value[82];

    /* names are copied before uppercasing, so literals are safe */
    var_set("LITVAR", "literal");
    FC_CHECK(var_get("LITVAR", value, sizeof(value)));
    FC_CHECK_EQ(str_cmp(value, "literal"), 0);

    var_set("LITVAR", "");
    FC_CHECK_EQ(var_get("LITVAR", value, sizeof(value)), 0);
}

int main(char* args) {
    (void)args;

    term_puts("VARTEST test\n");

    test_set_get();
    test_remove();
    test_unknown();
    test_limits();
    test_small_buffer();
    test_literal_name();

    return fc_summary();
}
