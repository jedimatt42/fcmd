#include "fctest.h"

/*
 * I8 - interactive terminal input, drop-down bar, and the identify hook.
 *
 * Covers: term_gets, term_kscan, term_drop_down, term_set_identify_hook.
 */

static volatile unsigned int identify_calls = 0;
static volatile unsigned int identify_flag = 0;

static void on_identify(int flag) {
    identify_calls++;
    identify_flag = (unsigned int)flag;
}

static void test_identify_hook(void) {
    term_set_identify_hook(on_identify);

    /* CSI 6n asks the terminal to report the cursor position; ForceCommand
       answers by invoking the identify hook with the 0x8000 marker set. */
    term_putc(27);
    term_putc('[');
    term_putc('6');
    term_putc('n');

    FC_CHECK_EQ(identify_calls, 1);
    FC_CHECK((identify_flag & 0x8000) != 0);
    term_putc('\n');

    term_set_identify_hook(0);
}

static void test_gets(void) {
    char buf[32];

    str_set(buf, 0, sizeof(buf));
    term_puts("Type some text (try backspace) and press ENTER: ");
    term_gets(buf, 30, 1);
    term_putc('\n');
    term_puts("You typed: ");
    term_puts(buf);
    term_putc('\n');

    FC_CHECK(str_len(buf) < (int)sizeof(buf));
    FC_OBSERVE("'You typed:' should match what you entered");
}

static void test_kscan(void) {
    unsigned int key = 0;

    term_puts("Press any key to test term_kscan: ");
    do {
        key = term_kscan(5);
    } while ((KSCAN_STATUS & KSCAN_MASK) == 0);

    term_putc('\n');
    term_puts("key code = ");
    term_puts(str_from_uint(key));
    term_putc('\n');
    FC_OBSERVE("key code should be the mode-5 scan code of the key you pressed");
}

static void test_drop_down(void) {
    term_cls();
    term_drop_down(3);
    term_gotoxy(1, 5);
    term_puts("drop down occupies the top 4 rows");
    FC_OBSERVE("three framed blank rows with a bar on the fourth, spanning the width");
    FC_WAIT("press a key to continue");
}

int main(char* args) {
    (void)args;

    term_puts("TERMREAD test\n");

    test_identify_hook();
    test_gets();
    test_kscan();
    test_drop_down();

    return fc_summary();
}
