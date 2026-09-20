#include "fctest.h"

/*
 * I2 - string utility functions.
 *
 * Covers: str_len, str_cmp, str_cmp_icase, str_copy, str_ncopy, str_cat,
 *         str_set, str_index_of, str_last_index_of, str_startswith,
 *         str_endswith, str_to_int, str_token, str_token_next,
 *         str_token_peek.
 *
 * Note: str_token and str_token_peek use a shared cursor, so they are driven
 * in sequence. str_token mutates its input in place, so mutable buffers are
 * used for it.
 */

static void test_len_cmp(void) {
    FC_CHECK_EQ(str_len(""), 0);
    FC_CHECK_EQ(str_len("a"), 1);
    FC_CHECK_EQ(str_len("hello"), 5);

    FC_CHECK_EQ(str_cmp("abc", "abc"), 0);
    FC_CHECK(str_cmp("abc", "abd") != 0);
    FC_CHECK(str_cmp("abc", "ab") != 0);
    FC_CHECK(str_cmp("ab", "abc") != 0);
    FC_CHECK_EQ(str_cmp("", ""), 0);

    FC_CHECK_EQ(str_cmp_icase("Hello", "hELLo"), 0);
    FC_CHECK(str_cmp_icase("abc", "abd") != 0);
    FC_CHECK(str_cmp_icase("abc", "ab") != 0);
    FC_CHECK_EQ(str_cmp_icase("", ""), 0);
}

static void test_copy_cat(void) {
    char buf[16];
    char src[] = "abcdef";
    char* r;

    r = str_copy(buf, "hi");
    FC_CHECK(r == buf);
    FC_CHECK_EQ(str_cmp(buf, "hi"), 0);

    r = str_cat(buf, " there");
    FC_CHECK(r == buf);
    FC_CHECK_EQ(str_cmp(buf, "hi there"), 0);

    str_set(buf, 'x', 4);
    FC_CHECK_EQ(buf[0], 'x');
    FC_CHECK_EQ(buf[3], 'x');
    /* str_set does not null terminate; a sentinel past the limit is untouched */
    buf[4] = 'z';
    str_set(buf, 'y', 4);
    FC_CHECK_EQ(buf[4], 'z');

    r = str_ncopy(buf, src, 3);
    FC_CHECK(r == buf);
    FC_CHECK_EQ(str_len(buf), 3);
    FC_CHECK_EQ(str_cmp(buf, "abc"), 0);

    str_ncopy(buf, src, 0);
    FC_CHECK_EQ(str_len(buf), 0);
    FC_CHECK_EQ(buf[0], 0);

    str_ncopy(buf, src, 16);
    FC_CHECK_EQ(str_cmp(buf, "abcdef"), 0);
}

static void test_search(void) {
    FC_CHECK_EQ(str_index_of("banana", 'b'), 0);
    FC_CHECK_EQ(str_index_of("banana", 'a'), 1);
    FC_CHECK_EQ(str_index_of("banana", 'z'), -1);
    FC_CHECK_EQ(str_index_of("", 'a'), -1);

    FC_CHECK_EQ(str_last_index_of("banana", 'a', 5), 5);
    FC_CHECK_EQ(str_last_index_of("banana", 'a', 3), 3);
    FC_CHECK_EQ(str_last_index_of("banana", 'z', 5), -1);
}

static void test_affix(void) {
    FC_CHECK_EQ(str_startswith("hello world", "hello"), 1);
    FC_CHECK_EQ(str_startswith("hello", "hello world"), 0);
    FC_CHECK_EQ(str_startswith("hello", ""), 1);
    FC_CHECK_EQ(str_startswith("hello", "x"), 0);

    FC_CHECK_EQ(str_endswith("hello world", "world"), 1);
    FC_CHECK_EQ(str_endswith("hello", "hello world"), 0);
    FC_CHECK_EQ(str_endswith("hello", ""), 1);
    FC_CHECK_EQ(str_endswith("hello", "x"), 0);
}

static void test_to_int(void) {
    FC_CHECK_EQ(str_to_int("0"), 0);
    FC_CHECK_EQ(str_to_int("42"), 42);
    FC_CHECK_EQ(str_to_int(" 42"), 42);
    FC_CHECK_EQ(str_to_int("-7"), -7);
    FC_CHECK_EQ(str_to_int("+9"), 9);
    FC_CHECK_EQ(str_to_int("12abc"), 12);
    FC_CHECK_EQ(str_to_int("abc"), 0);
    FC_CHECK_EQ(str_to_int(""), 0);
}

static void test_token(void) {
    char text[] = "alpha,beta,,gamma";
    char* t;

    t = str_token(text, ',');
    FC_CHECK(t != 0);
    FC_CHECK_EQ(str_cmp(t, "alpha"), 0);

    t = str_token(0, ',');
    FC_CHECK_EQ(str_cmp(t, "beta"), 0);

    /* consecutive delimiters are skipped */
    t = str_token(0, ',');
    FC_CHECK_EQ(str_cmp(t, "gamma"), 0);

    t = str_token(0, ',');
    FC_CHECK(t == 0);

    /* trailing delimiter yields no final token */
    char trailing[] = "only,";
    t = str_token(trailing, ',');
    FC_CHECK_EQ(str_cmp(t, "only"), 0);
    t = str_token(0, ',');
    FC_CHECK(t == 0);
}

static void test_token_peek(void) {
    char text[] = "one two three";
    char* t;
    char* p;

    t = str_token(text, ' ');
    FC_CHECK_EQ(str_cmp(t, "one"), 0);

    p = str_token_peek(0, ' ');
    FC_CHECK_EQ(str_cmp(p, "two"), 0);

    /* peek does not advance the cursor */
    p = str_token_peek(0, ' ');
    FC_CHECK_EQ(str_cmp(p, "two"), 0);

    t = str_token(0, ' ');
    FC_CHECK_EQ(str_cmp(t, "two"), 0);
    t = str_token(0, ' ');
    FC_CHECK_EQ(str_cmp(t, "three"), 0);
    t = str_token(0, ' ');
    FC_CHECK(t == 0);
}

static void test_token_next(void) {
    char src[] = "a;bb;ccc";
    char dst[16];
    char* next;

    next = str_token_next(dst, src, ';');
    FC_CHECK_EQ(str_cmp(dst, "a"), 0);
    FC_CHECK(next != 0);

    next = str_token_next(dst, next, ';');
    FC_CHECK_EQ(str_cmp(dst, "bb"), 0);

    next = str_token_next(dst, next, ';');
    FC_CHECK_EQ(str_cmp(dst, "ccc"), 0);
    FC_CHECK(next == 0);

    /* a null source clears the destination and returns null */
    dst[0] = 'x';
    next = str_token_next(dst, 0, ';');
    FC_CHECK_EQ(dst[0], 0);
    FC_CHECK(next == 0);
}

int main(char* args) {
    (void)args;

    term_puts("STRTEST test\n");

    test_len_cmp();
    test_copy_cat();
    test_search();
    test_affix();
    test_to_int();
    test_token();
    test_token_peek();
    test_token_next();

    return fc_summary();
}
