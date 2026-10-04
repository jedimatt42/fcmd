#include "fctest.h"

/*
 * I26 - line primitives (AUTO + OBSERVE).
 *
 * Covers gfx_line in normal, box, fill, and combined styles, out-of-range
 * coordinates and invalid styles/colors, with reads back through gfx_point.
 *
 * Each mode is shown interactively: in TEXT40 the test says what to look for
 * and waits for a key; then it enters the mode and draws; a second key returns
 * to TEXT40, where the AUTO checks for that mode are printed.
 *
 * Modes:
 *   GRAPHICS2   - always available; all styles and error paths
 *   MULTICOLOR  - always available; normal line
 *   GRAPHICS4   - V9938/V9958 only; normal line
 */

static void to_text(void) {
    gfx_screen(GFX_MODE_TEXT40, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    term_cls();
}

/* In TEXT40: describe the picture and wait for the user to start the drawing. */
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
    int rc_norm, rc_range_a, rc_range_b, rc_style_bad, rc_color_bad;
    int h_a, h_b, h_c, h_off;
    int d_a, d_b, d_c, d_off;
    int b_tl, b_tr, b_bl, b_br, b_t, b_b, b_l, b_r, b_in;
    int f_a, f_b, f_c, f_d, f_off;
    int c_a, c_b, c_c;

    announce("GRAPHICS2",
             "a horizontal line, a diagonal line, a hollow box, and two "
             "filled rectangles (FILL and BOX|FILL look identical)");

    gfx_screen(GFX_MODE_GRAPHICS2, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_get_info(&info);

    /* Normal horizontal line. */
    gfx_clear(GFX_COLOR_DEFAULT);
    rc_norm = gfx_line(10, 20, 50, 20, 5, GFX_LINE_NORMAL, GFX_OP_PSET);
    h_a = sample(10, 20);
    h_b = sample(30, 20);
    h_c = sample(50, 20);
    h_off = sample(30, 10);

    /* Normal diagonal line (slope 1). */
    gfx_line(10, 40, 30, 60, 5, GFX_LINE_NORMAL, GFX_OP_PSET);
    d_a = sample(10, 40);
    d_b = sample(20, 50);
    d_c = sample(30, 60);
    d_off = sample(10, 60);

    /* Box outline (not filled). */
    gfx_line(80, 40, 120, 60, 5, GFX_LINE_BOX, GFX_OP_PSET);
    b_tl = sample(80, 40);
    b_tr = sample(120, 40);
    b_bl = sample(80, 60);
    b_br = sample(120, 60);
    b_t = sample(100, 40);
    b_b = sample(100, 60);
    b_l = sample(80, 50);
    b_r = sample(120, 50);
    b_in = sample(100, 50);

    /* Filled rectangle. */
    gfx_line(40, 100, 80, 120, 5, GFX_LINE_FILL, GFX_OP_PSET);
    f_a = sample(40, 100);
    f_b = sample(80, 120);
    f_c = sample(60, 110);
    f_d = sample(45, 115);
    f_off = sample(39, 110);

    /* Box combined with fill. */
    gfx_line(120, 100, 160, 120, 5, GFX_LINE_BOX | GFX_LINE_FILL, GFX_OP_PSET);
    c_a = sample(120, 100);
    c_b = sample(160, 120);
    c_c = sample(140, 110);

    /* Error paths. */
    rc_range_a = gfx_line(10, 20, info.width, 20, 5, GFX_LINE_NORMAL,
                          GFX_OP_PSET);
    rc_range_b = gfx_line(-1, 20, 10, 20, 5, GFX_LINE_NORMAL, GFX_OP_PSET);
    rc_style_bad = gfx_line(10, 20, 30, 20, 5, 0x80, GFX_OP_PSET);
    rc_color_bad = gfx_line(10, 20, 30, 20, 99, GFX_LINE_NORMAL, GFX_OP_PSET);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_norm, GFX_OK);
    FC_CHECK_EQ(h_a, 5);
    FC_CHECK_EQ(h_b, 5);
    FC_CHECK_EQ(h_c, 5);
    FC_CHECK_EQ(h_off, 0);
    FC_CHECK_EQ(d_a, 5);
    FC_CHECK_EQ(d_b, 5);
    FC_CHECK_EQ(d_c, 5);
    FC_CHECK_EQ(d_off, 0);

    FC_CHECK_EQ(b_tl, 5);
    FC_CHECK_EQ(b_tr, 5);
    FC_CHECK_EQ(b_bl, 5);
    FC_CHECK_EQ(b_br, 5);
    FC_CHECK_EQ(b_t, 5);
    FC_CHECK_EQ(b_b, 5);
    FC_CHECK_EQ(b_l, 5);
    FC_CHECK_EQ(b_r, 5);
    FC_CHECK_EQ(b_in, 0);

    FC_CHECK_EQ(f_a, 5);
    FC_CHECK_EQ(f_b, 5);
    FC_CHECK_EQ(f_c, 5);
    FC_CHECK_EQ(f_d, 5);
    FC_CHECK_EQ(f_off, 0);

    FC_CHECK_EQ(c_a, 5);
    FC_CHECK_EQ(c_b, 5);
    FC_CHECK_EQ(c_c, 5);

    FC_CHECK_EQ(rc_range_a, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_range_b, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_style_bad, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_color_bad, GFX_ERR_RANGE);
}

static void test_multicolor(void) {
    int a, b, c, off;

    announce("MULTICOLOR", "a single horizontal line");

    gfx_screen(GFX_MODE_MULTICOLOR, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_clear(GFX_COLOR_DEFAULT);

    gfx_line(10, 10, 20, 10, 5, GFX_LINE_NORMAL, GFX_OP_PSET);
    a = sample(10, 10);
    b = sample(15, 10);
    c = sample(20, 10);
    off = sample(15, 12);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(a, 5);
    FC_CHECK_EQ(b, 5);
    FC_CHECK_EQ(c, 5);
    FC_CHECK_EQ(off, 0);
}

static void test_graphics4(void) {
    int a, b, c, off;

    announce("GRAPHICS4", "a diagonal line");

    gfx_screen(GFX_MODE_GRAPHICS4, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_clear(GFX_COLOR_DEFAULT);

    gfx_line(10, 10, 30, 30, 5, GFX_LINE_NORMAL, GFX_OP_PSET);
    a = sample(10, 10);
    b = sample(20, 20);
    c = sample(30, 30);
    off = sample(10, 30);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(a, 5);
    FC_CHECK_EQ(b, 5);
    FC_CHECK_EQ(c, 5);
    FC_CHECK_EQ(off, 0);
}

int main(char* args) {
    struct GfxInformation info;
    int vdp;
    int wrong_mode;
    int result;

    (void)args;

    to_text();
    term_puts("GFXLINES test\n");
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    vdp = info.vdp_type;

    /* Text mode has no pixel plane. */
    wrong_mode = gfx_line(0, 0, 10, 10, 1, GFX_LINE_NORMAL, GFX_OP_PSET);

    test_graphics2();
    test_multicolor();
    if (vdp == VDP_9938 || vdp == VDP_9958) {
        test_graphics4();
    } else {
        FC_SKIP("GRAPHICS4 requires V9938/V9958");
    }

    FC_CHECK_EQ(wrong_mode, GFX_ERR_WRONG_MODE);

    to_text();
    term_puts("[OBSERVE] all line shapes drawn as described\n");
    result = fc_summary();
    FC_WAIT("press a key to return to ForceCommand");
    return result;
}
