#include "fctest.h"

/*
 * I27 - circle and ellipse primitives (AUTO + OBSERVE).
 *
 * Covers gfx_circle, gfx_ellipse and the invalid-argument error codes, with
 * reads back through gfx_point.
 *
 * Each mode is shown interactively: in TEXT40 the test says what to look for
 * and waits for a key; then it enters the mode and draws; a second key returns
 * to TEXT40, where the AUTO checks for that mode are printed.
 *
 * Modes:
 *   GRAPHICS2   - always available; circle + ellipse and all error paths
 *   MULTICOLOR  - always available; small circle
 *   GRAPHICS4   - V9938/V9958 only; circle + ellipse
 *   GRAPHICS5   - V9938/V9958 only; 512-wide aspect correction
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

static int draw_color(const struct GfxInformation* info) {
    return (info->colors > 7) ? 5 : (info->colors - 1);
}

static void test_graphics2(void) {
    struct GfxInformation info;
    int dc;
    int rc_circle, rc_ellipse, rc_inv_a, rc_inv_b, rc_inv_c, rc_range, rc_color;
    int c_e, c_w, c_s, c_n, c_in;
    int e_e, e_w, e_s, e_n, e_in;

    announce("GRAPHICS2", "a circle on the left and an ellipse on the right");

    gfx_screen(GFX_MODE_GRAPHICS2, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_get_info(&info);
    gfx_clear(GFX_COLOR_DEFAULT);
    dc = draw_color(&info);

    rc_circle = gfx_circle(64, 64, 40, dc, GFX_OP_PSET);
    c_e = sample(104, 64);
    c_w = sample(24, 64);
    c_s = sample(64, 104);
    c_n = sample(64, 24);
    c_in = sample(64, 64);

    rc_ellipse = gfx_ellipse(192, 64, 40, 20, dc, GFX_OP_PSET);
    e_e = sample(232, 64);
    e_w = sample(152, 64);
    e_s = sample(192, 84);
    e_n = sample(192, 44);
    e_in = sample(192, 64);

    rc_inv_a = gfx_circle(64, 64, 0, dc, GFX_OP_PSET);
    rc_inv_b = gfx_ellipse(64, 64, 0, 10, dc, GFX_OP_PSET);
    rc_inv_c = gfx_ellipse(64, 64, 10, -1, dc, GFX_OP_PSET);
    rc_range = gfx_circle(-5, 64, 100, dc, GFX_OP_PSET);
    rc_color = gfx_circle(64, 64, 10, 99, GFX_OP_PSET);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_circle, GFX_OK);
    FC_CHECK_EQ(c_e, dc);
    FC_CHECK_EQ(c_w, dc);
    FC_CHECK_EQ(c_s, dc);
    FC_CHECK_EQ(c_n, dc);
    FC_CHECK_EQ(c_in, 0);
    FC_CHECK_EQ(rc_ellipse, GFX_OK);
    FC_CHECK_EQ(e_e, dc);
    FC_CHECK_EQ(e_w, dc);
    FC_CHECK_EQ(e_s, dc);
    FC_CHECK_EQ(e_n, dc);
    FC_CHECK_EQ(e_in, 0);
    FC_CHECK_EQ(rc_inv_a, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_inv_b, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_inv_c, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_range, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_color, GFX_ERR_RANGE);
}

static void test_multicolor(void) {
    struct GfxInformation info;
    int dc;
    int rc, e, w, s, n, in;

    announce("MULTICOLOR", "a small circle");

    gfx_screen(GFX_MODE_MULTICOLOR, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_get_info(&info);
    gfx_clear(GFX_COLOR_DEFAULT);
    dc = draw_color(&info);

    rc = gfx_circle(32, 24, 8, dc, GFX_OP_PSET);
    e = sample(40, 24);
    w = sample(24, 24);
    s = sample(32, 32);
    n = sample(32, 16);
    in = sample(32, 24);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc, GFX_OK);
    FC_CHECK_EQ(e, dc);
    FC_CHECK_EQ(w, dc);
    FC_CHECK_EQ(s, dc);
    FC_CHECK_EQ(n, dc);
    FC_CHECK_EQ(in, 0);
}

static void test_graphics4(void) {
    int rc_circle, rc_ellipse;
    int c_e, c_w, c_s, c_n, c_in;
    int e_e, e_w, e_s, e_n, e_in;

    announce("GRAPHICS4", "a circle and an ellipse");

    gfx_screen(GFX_MODE_GRAPHICS4, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_clear(GFX_COLOR_DEFAULT);

    rc_circle = gfx_circle(64, 64, 40, 5, GFX_OP_PSET);
    c_e = sample(104, 64);
    c_w = sample(24, 64);
    c_s = sample(64, 104);
    c_n = sample(64, 24);
    c_in = sample(64, 64);

    rc_ellipse = gfx_ellipse(160, 64, 40, 20, 5, GFX_OP_PSET);
    e_e = sample(200, 64);
    e_w = sample(120, 64);
    e_s = sample(160, 84);
    e_n = sample(160, 44);
    e_in = sample(160, 64);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_circle, GFX_OK);
    FC_CHECK_EQ(c_e, 5);
    FC_CHECK_EQ(c_w, 5);
    FC_CHECK_EQ(c_s, 5);
    FC_CHECK_EQ(c_n, 5);
    FC_CHECK_EQ(c_in, 0);
    FC_CHECK_EQ(rc_ellipse, GFX_OK);
    FC_CHECK_EQ(e_e, 5);
    FC_CHECK_EQ(e_w, 5);
    FC_CHECK_EQ(e_s, 5);
    FC_CHECK_EQ(e_n, 5);
    FC_CHECK_EQ(e_in, 0);
}

static void test_graphics5(void) {
    struct GfxInformation info;
    int dc;
    int rc_circle, rc_ellipse;
    int c_e, c_w, c_s, c_n, c_in;
    int e_e, e_w, e_s, e_n, e_in;

    announce("GRAPHICS5",
             "a circle (drawn twice as wide in pixels to look round) "
             "and an ellipse");

    gfx_screen(GFX_MODE_GRAPHICS5, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_get_info(&info);
    gfx_clear(GFX_COLOR_DEFAULT);
    dc = draw_color(&info);

    /* 512-wide: gfx_circle uses radius_x = 2*radius. */
    rc_circle = gfx_circle(256, 106, 30, dc, GFX_OP_PSET);
    c_e = sample(316, 106);
    c_w = sample(196, 106);
    c_s = sample(256, 136);
    c_n = sample(256, 76);
    c_in = sample(256, 106);

    rc_ellipse = gfx_ellipse(400, 106, 40, 20, dc, GFX_OP_PSET);
    e_e = sample(440, 106);
    e_w = sample(360, 106);
    e_s = sample(400, 126);
    e_n = sample(400, 86);
    e_in = sample(400, 106);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_circle, GFX_OK);
    FC_CHECK_EQ(c_e, dc);
    FC_CHECK_EQ(c_w, dc);
    FC_CHECK_EQ(c_s, dc);
    FC_CHECK_EQ(c_n, dc);
    FC_CHECK_EQ(c_in, 0);
    FC_CHECK_EQ(rc_ellipse, GFX_OK);
    FC_CHECK_EQ(e_e, dc);
    FC_CHECK_EQ(e_w, dc);
    FC_CHECK_EQ(e_s, dc);
    FC_CHECK_EQ(e_n, dc);
    FC_CHECK_EQ(e_in, 0);
}

int main(char* args) {
    struct GfxInformation info;
    int vdp;
    int wrong_circle, wrong_ellipse;
    int result;

    (void)args;

    to_text();
    term_puts("GFXCIRCLE test\n");
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    vdp = info.vdp_type;

    wrong_circle = gfx_circle(0, 0, 5, 1, GFX_OP_PSET);
    wrong_ellipse = gfx_ellipse(0, 0, 5, 3, 1, GFX_OP_PSET);

    test_graphics2();
    test_multicolor();
    if (vdp == VDP_9938 || vdp == VDP_9958) {
        test_graphics4();
        test_graphics5();
    } else {
        FC_SKIP("GRAPHICS4/GRAPHICS5 require V9938/V9958");
    }

    FC_CHECK_EQ(wrong_circle, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(wrong_ellipse, GFX_ERR_WRONG_MODE);

    to_text();
    term_puts("[OBSERVE] circle and ellipse shapes drawn as described\n");
    result = fc_summary();
    FC_WAIT("press a key to return to ForceCommand");
    return result;
}
