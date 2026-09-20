#include "fctest.h"

/*
 * I3 - numeric and BASIC string formatting.
 *
 * Covers: str_from_uint, str_from_float, str_from_basic, plus hexadecimal
 *         edge cases shared with hex_from_uint.
 *
 * str_from_float output format is only lightly asserted (zero, sign, leading
 * digit) because the radix-100 to decimal conversion is non-trivial; the
 * representative values are printed for human observation.
 */

struct UintCase {
    unsigned int value;
    const char* text;
};

static const struct UintCase uint_cases[] = {
    { 0,     "0" },
    { 1,     "1" },
    { 9,     "9" },
    { 10,    "10" },
    { 99,    "99" },
    { 100,   "100" },
    { 1000,  "1000" },
    { 12345, "12345" },
    { 65535, "65535" }
};

static void test_from_uint(void) {
    unsigned int i;
    for (i = 0; i < sizeof(uint_cases) / sizeof(uint_cases[0]); i++) {
        char* got = str_from_uint(uint_cases[i].value);
        term_puts("uint ");
        term_puts(uint_cases[i].text);
        term_puts(" -> ");
        term_puts(got);
        term_putc('\n');
        FC_CHECK_EQ(str_cmp(got, uint_cases[i].text), 0);
    }
}

static void test_hex_edges(void) {
    FC_CHECK_EQ(str_cmp(hex_from_uint(0x0000), "0000"), 0);
    FC_CHECK_EQ(str_cmp(hex_from_uint(0x000A), "000A"), 0);
    FC_CHECK_EQ(str_cmp(hex_from_uint(0x00FF), "00FF"), 0);
    FC_CHECK_EQ(str_cmp(hex_from_uint(0xFFFF), "FFFF"), 0);
}

static void test_from_basic(void) {
    char buf[16];
    char basic[8];
    int r;

    basic[0] = 5;
    basic[1] = 'H';
    basic[2] = 'e';
    basic[3] = 'l';
    basic[4] = 'l';
    basic[5] = 'o';

    r = str_from_basic(basic, buf);
    FC_CHECK_EQ(r, 5);
    FC_CHECK_EQ(str_cmp(buf, "Hello"), 0);

    basic[0] = 0;
    r = str_from_basic(basic, buf);
    FC_CHECK_EQ(r, 0);
    FC_CHECK_EQ(buf[0], 0);

    basic[0] = 1;
    basic[1] = 'x';
    r = str_from_basic(basic, buf);
    FC_CHECK_EQ(r, 1);
    FC_CHECK_EQ(str_cmp(buf, "x"), 0);
}

static void show_float(double v) {
    term_puts("  ");
    term_puts(str_from_float(v));
    term_putc('\n');
}

static void test_from_float(void) {
    char* s;

    s = str_from_float(0.0);
    FC_CHECK_EQ(str_cmp(s, "0"), 0);

    s = str_from_float(-1.0);
    FC_CHECK_EQ(s[0], '-');

    s = str_from_float(1.0);
    FC_CHECK(s[0] >= '0' && s[0] <= '9');

    term_puts("[OBSERVE] str_from_float values:\n");
    show_float(0.5);
    show_float(1.0);
    show_float(-1.0);
    show_float(3.14159);
    show_float(100.0);
    show_float(12345.0);
    FC_OBSERVE("each printed value should read as a sensible decimal number");
}

int main(char* args) {
    (void)args;

    term_puts("STRFMT test\n");

    test_from_uint();
    test_hex_edges();
    test_from_basic();
    test_from_float();

    return fc_summary();
}
