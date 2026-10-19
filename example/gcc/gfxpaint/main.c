#include "fctest.h"

/*
 * I28 - paint and vector draw (AUTO + OBSERVE).
 *
 * Covers gfx_paint (flood fill bounded by a border color) and gfx_draw
 * (MSX-style relative vector commands), plus their error codes.
 *
 * Each mode is shown interactively: in TEXT40 the test says what to look for
 * and waits for a key; then it enters the mode and draws; a second key returns
 * to TEXT40, where the AUTO checks for that mode are printed.
 *
 * Modes:
 *   GRAPHICS2   - always available; paint and the full command grammar
 *   MULTICOLOR  - always available; paint (per-dot colors) and a line
 *   GRAPHICS4   - V9938/V9958 only; paint with a preserved border and a diagonal
 *
 * Note: in GRAPHICS2 the color is selected per 8x8 group, so filling the
 * interior also sets the border pixels' group color; the border is only
 * asserted to keep its color in the per-pixel modes (MULTICOLOR/GRAPHICS4).
 */

static void to_text(void) {
    gfx_screen(GFX_MODE_TEXT40, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    term_cls();
}

static void announce(const char* mode, const char* what) {
    term_puts("[OBSERVE] ");
    term_puts(mode);
    term_puts(": ");
    term_puts(what);
    term_putc('\n');
    FC_WAIT("press a key to draw, then a key again to continue");
}

static int sample(int x, int y) {
    int c = -1;
    gfx_point(x, y, &c);
    return c;
}

static void test_graphics2(void) {
    struct GfxInformation info;
    int rc_paint, rc_repeat, rc_draw;
    int p_in1, p_in2, p_out;
    int d_on, d_off, d_vert, d_diag, d_move, d_after, d_def_a, d_def_b;
    int rc_bad_cmd, rc_missing, rc_null, rc_bad_color;
    int rc_paint_color, rc_paint_border, rc_busy;

    announce("GRAPHICS2",
             "a hollow box filled with color, then lines drawn by vector "
             "commands (M/R/D/E/B)");

    gfx_screen(GFX_MODE_GRAPHICS2, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_get_info(&info);

    /* Flood fill: border 7, fill 5. Keep the region small enough for the
       fixed 256-entry paint stack. */
    gfx_clear(GFX_COLOR_DEFAULT);
    gfx_line(20, 20, 26, 26, 7, GFX_LINE_BOX, GFX_OP_PSET);
    rc_paint = gfx_paint(23, 23, 5, 7, GFX_OP_PSET);
    p_in1 = sample(23, 23);
    p_in2 = sample(22, 22);
    p_out = sample(10, 10);
    rc_repeat = gfx_paint(23, 23, 5, 7, GFX_OP_PSET);

    /* Vector commands. */
    gfx_clear(GFX_COLOR_DEFAULT);
    rc_draw = gfx_draw("M10,10 R40", 5, GFX_OP_PSET);
    d_on = sample(30, 10);
    d_off = sample(30, 15);

    gfx_draw("M10,30 D20", 5, GFX_OP_PSET);
    d_vert = sample(10, 40);

    gfx_draw("M10,70 E20", 5, GFX_OP_PSET);
    d_diag = sample(20, 60);

    gfx_draw("M80,80 BR20 R20", 5, GFX_OP_PSET);
    d_move = sample(90, 80);
    d_after = sample(110, 80);

    gfx_draw("M10,100 R", 5, GFX_OP_PSET);
    d_def_a = sample(10, 100);
    d_def_b = sample(11, 100);

    rc_bad_cmd = gfx_draw("Z10", 5, GFX_OP_PSET);
    rc_missing = gfx_draw("M", 5, GFX_OP_PSET);
    rc_null = gfx_draw(0, 5, GFX_OP_PSET);
    rc_bad_color = gfx_draw("R10", 99, GFX_OP_PSET);
    rc_paint_color = gfx_paint(0, 0, 99, 7, GFX_OP_PSET);
    rc_paint_border = gfx_paint(0, 0, 5, 99, GFX_OP_PSET);

    /* An unbounded fill exhausts the paint stack. */
    gfx_clear(GFX_COLOR_DEFAULT);
    rc_busy = gfx_paint(128, 96, 5, 6, GFX_OP_PSET);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_paint, GFX_OK);
    FC_CHECK_EQ(p_in1, 5);
    FC_CHECK_EQ(p_in2, 5);
    FC_CHECK_EQ(p_out, 0);
    FC_CHECK_EQ(rc_repeat, GFX_OK);
    FC_CHECK_EQ(rc_draw, GFX_OK);
    FC_CHECK_EQ(d_on, 5);
    FC_CHECK_EQ(d_off, 0);
    FC_CHECK_EQ(d_vert, 5);
    FC_CHECK_EQ(d_diag, 5);
    FC_CHECK_EQ(d_move, 0);
    FC_CHECK_EQ(d_after, 5);
    FC_CHECK_EQ(d_def_a, 5);
    FC_CHECK_EQ(d_def_b, 5);
    FC_CHECK_EQ(rc_bad_cmd, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_missing, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_null, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_bad_color, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_paint_color, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_paint_border, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_busy, GFX_ERR_BUSY);
    (void)info;
}

static void test_multicolor(void) {
    int rc_paint, rc_draw;
    int p_in, p_border, p_out, d_on;

    announce("MULTICOLOR",
             "a hollow box filled with color (border keeps its color) and "
             "a vector line");

    gfx_screen(GFX_MODE_MULTICOLOR, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_clear(GFX_COLOR_DEFAULT);

    gfx_line(10, 10, 16, 16, 7, GFX_LINE_BOX, GFX_OP_PSET);
    rc_paint = gfx_paint(13, 13, 5, 7, GFX_OP_PSET);
    p_in = sample(13, 13);
    p_border = sample(10, 13);
    p_out = sample(5, 5);

    rc_draw = gfx_draw("M5,30 R10", 5, GFX_OP_PSET);
    d_on = sample(10, 30);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_paint, GFX_OK);
    FC_CHECK_EQ(p_in, 5);
    FC_CHECK_EQ(p_border, 7);
    FC_CHECK_EQ(p_out, 0);
    FC_CHECK_EQ(rc_draw, GFX_OK);
    FC_CHECK_EQ(d_on, 5);
}

static void test_graphics4(void) {
    int rc_paint, rc_draw;
    int p_in, p_border, p_out, d_on;

    announce("GRAPHICS4",
             "a hollow box filled with color (border keeps its color) and "
             "a diagonal vector line");

    gfx_screen(GFX_MODE_GRAPHICS4, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_clear(GFX_COLOR_DEFAULT);

    gfx_line(20, 20, 26, 26, 7, GFX_LINE_BOX, GFX_OP_PSET);
    rc_paint = gfx_paint(23, 23, 5, 7, GFX_OP_PSET);
    p_in = sample(23, 23);
    p_border = sample(20, 23);
    p_out = sample(10, 10);

    rc_draw = gfx_draw("M100,160 E30", 5, GFX_OP_PSET);
    d_on = sample(115, 145);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_paint, GFX_OK);
    FC_CHECK_EQ(p_in, 5);
    FC_CHECK_EQ(p_border, 7);
    FC_CHECK_EQ(p_out, 0);
    FC_CHECK_EQ(rc_draw, GFX_OK);
    FC_CHECK_EQ(d_on, 5);
}

int main(char* args) {
    struct GfxInformation info;
    int vdp;
    int wrong_paint, wrong_draw;
    int result;

    (void)args;

    to_text();
    term_puts("GFXPAINT test\n");
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    vdp = info.vdp_type;

    wrong_paint = gfx_paint(0, 0, 1, 2, GFX_OP_PSET);
    wrong_draw = gfx_draw("R10", 1, GFX_OP_PSET);

    test_graphics2();
    test_multicolor();
    if (vdp == VDP_9938 || vdp == VDP_9958) {
        test_graphics4();
    } else {
        FC_SKIP("GRAPHICS4 requires V9938/V9958");
    }

    FC_CHECK_EQ(wrong_paint, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(wrong_draw, GFX_ERR_WRONG_MODE);

    to_text();
    term_puts("[OBSERVE] paint fills and vector lines drawn as described\n");
    result = fc_summary();
    FC_WAIT("press a key to return to ForceCommand");
    return result;
}
