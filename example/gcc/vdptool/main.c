#include "fctest.h"
#include <ioports.h>

/*
 * I9 - VDP cursor address and character writes.
 *
 * Covers: vdp_cursor_addr, vdp_setchar.
 *
 * The screen is cleared first so all rows are known. The observed cell is
 * placed on its own row and the cursor is moved above it before any status
 * text is printed, so the test output never covers the character being
 * observed. The original byte is saved and restored.
 */

/* the observed cell lives here; status output stays well above it */
#define OBSERVE_ROW 20
#define MASKED_ROW  22

static void test_cursor_addr(void) {
    unsigned int a, b, c;

    term_gotoxy(1, 1);
    a = vdp_cursor_addr();

    term_gotoxy(2, 1);
    b = vdp_cursor_addr();
    /* adjacent columns differ by exactly one character cell */
    FC_CHECK_EQ(b - a, 1);

    term_gotoxy(1, 2);
    c = vdp_cursor_addr();
    /* the next row adds the full row stride (32, 40, or 80) */
    FC_CHECK(c - a > 1);
}

static void test_setchar(void) {
    unsigned int addr;
    unsigned char old, got;

    term_gotoxy(1, OBSERVE_ROW);
    addr = vdp_cursor_addr();
    old = vdp_readchar(addr);

    vdp_setchar(addr, 'Q');
    got = vdp_readchar(addr);

    /* print status well above the observed cell */
    term_gotoxy(1, 5);
    FC_CHECK_EQ(got, 'Q');
    FC_OBSERVE("a 'Q' should appear at row 20, column 1");
    FC_WAIT("press a key to restore the cell");

    vdp_setchar(addr, old);
    FC_CHECK_EQ(vdp_readchar(addr), old);
}

static void test_setchar_masked(void) {
    unsigned int addr;
    unsigned char old, got;

    /* a full byte value must round-trip through the data port */
    term_gotoxy(1, MASKED_ROW);
    addr = vdp_cursor_addr();
    old = vdp_readchar(addr);

    vdp_setchar(addr, 0x7f);
    got = vdp_readchar(addr);

    term_gotoxy(1, 10);
    FC_CHECK_EQ(got, 0x7f);

    vdp_setchar(addr, old);
    FC_CHECK_EQ(vdp_readchar(addr), old);
}

int main(char* args) {
    (void)args;

    /* establish a known screen so every row is blank and predictable */
    term_cls();
    term_puts("VDPTOOL test\n");

    test_cursor_addr();
    test_setchar();
    test_setchar_masked();

    term_gotoxy(1, 12);
    return fc_summary();
}
