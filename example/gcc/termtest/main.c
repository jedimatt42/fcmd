#include "fctest.h"

/*
 * I7 - terminal control.
 *
 * Covers: term_puts, term_putc, term_gotoxy, term_cls, term_set_text_color,
 *         term_set_bg_color, term_set_border_color.
 *
 * The color setters return the previous value, so the return codes are checked
 * automatically. Actual colors and positions are confirmed by observation.
 *
 * Coordinates are 1-based; term_gotoxy clamps out-of-range values.
 */

static unsigned int orig_fg;
static unsigned int orig_bg;

static void test_cls_and_gotoxy(void) {
    term_cls();
    FC_OBSERVE("screen should now be clear");

    term_gotoxy(1, 1);
    term_puts("top left");

    term_gotoxy(20, 5);
    term_puts("middle");

    FC_OBSERVE("'top left' at row 1 column 1, 'middle' at row 5 column 20");
}

static void test_putc(void) {
    term_gotoxy(1, 3);
    term_puts("putc: ");
    term_putc('A');
    term_putc('B');
    term_putc('C');
    term_putc('\n');
    FC_OBSERVE("row 3 should read: putc: ABC");
}

static void test_colors(void) {
    unsigned int old;

    term_cls();

    term_set_text_color(COLOR_LTGREEN);
    old = term_set_text_color(COLOR_MAGENTA);
    FC_CHECK_EQ(old, COLOR_LTGREEN);

    term_set_bg_color(COLOR_DKBLUE);
    old = term_set_bg_color(COLOR_BLACK);
    FC_CHECK_EQ(old, COLOR_DKBLUE);
    term_set_bg_color(COLOR_DKBLUE);

    term_set_text_color(COLOR_CYAN);
    term_gotoxy(1, 1);
    term_puts("cyan text");

    term_set_text_color(COLOR_WHITE);
    term_gotoxy(1, 2);
    term_puts("white text");

    FC_OBSERVE("80x30 F18A: row 1 cyan, row 2 white, dark blue background; "
               "global-color modes show all text in the last color");
}

static void test_border(void) {
    unsigned int r;

    r = term_set_border_color(COLOR_MAGENTA);
    FC_CHECK_EQ(r, 0);
    FC_OBSERVE("screen border should now be magenta");
    FC_WAIT("press a key to restore the border");

    term_set_border_color(COLOR_BLACK);
}

int main(char* args) {
    (void)args;

    term_puts("TERMTEST test\n");

    /* capture the current colors (and normalize) so return codes are
       predictable regardless of the startup mode */
    orig_fg = term_set_text_color(COLOR_WHITE);
    orig_bg = term_set_bg_color(COLOR_BLACK);

    test_cls_and_gotoxy();
    test_putc();
    test_colors();
    test_border();

    /* restore the caller's colors */
    term_set_bg_color(orig_bg);
    term_set_text_color(orig_fg);
    term_set_border_color(orig_bg & 0x0f);

    return fc_summary();
}
