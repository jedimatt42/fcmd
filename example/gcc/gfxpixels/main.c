#include "fctest.h"

/*
 * I25 - pixel primitives (AUTO).
 *
 * Covers gfx_set_cursor, gfx_get_cursor, gfx_pset, gfx_preset, gfx_point and
 * the GFX_OP_* operators with a pixel round-trip.
 *
 * Terminal output is inert while a graphics mode is active (FU2), so every
 * drawing operation and read-back is captured during the mode and checked
 * after returning to TEXT40.
 *
 * Modes:
 *   GRAPHICS2   - always available; full operator/cursor/range checks
 *   MULTICOLOR  - always available; round-trip and XOR
 *   GRAPHICS4   - V9938/V9958 only; adjacent-pixel nibble packing
 */

static void to_text(void) {
    gfx_screen(GFX_MODE_TEXT40, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    term_cls();
}

static void test_graphics2(void) {
    struct GfxInformation info;
    int v_ps = -1, v_pr = -1, v_or = -1, v_and = -1, v_xor = -1, v_def = -1;
    int cx = -1, cy = -1;
    int rc_pset, rc_pset_range, rc_point_range, rc_color_range;
    int rc_setcur_range, rc_getcur_null, rc_point_null;
    int dummy = 0;

    gfx_screen(GFX_MODE_GRAPHICS2, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_clear(GFX_COLOR_DEFAULT);
    gfx_get_info(&info);

    /* Each test pixel is in its own 8x8 color group (Graphics II). */
    rc_pset = gfx_pset(8, 8, 5, GFX_OP_PSET);
    gfx_point(8, 8, &v_ps);

    gfx_pset(16, 8, 5, GFX_OP_PSET);
    gfx_preset(16, 8, GFX_COLOR_DEFAULT, GFX_OP_PRESET);
    gfx_point(16, 8, &v_pr);

    gfx_pset(24, 8, 4, GFX_OP_PSET);
    gfx_pset(24, 8, 2, GFX_OP_OR);
    gfx_point(24, 8, &v_or);

    gfx_pset(32, 8, 7, GFX_OP_PSET);
    gfx_pset(32, 8, 3, GFX_OP_AND);
    gfx_point(32, 8, &v_and);

    gfx_pset(40, 8, 6, GFX_OP_PSET);
    gfx_pset(40, 8, 1, GFX_OP_XOR);
    gfx_point(40, 8, &v_xor);

    /* A default color uses the current foreground (1). */
    gfx_pset(48, 8, GFX_COLOR_DEFAULT, GFX_OP_PSET);
    gfx_point(48, 8, &v_def);

    /* gfx_pset advances the cursor; gfx_set_cursor moves it explicitly. */
    gfx_set_cursor(200, 100);
    gfx_pset(208, 100, 2, GFX_OP_PSET);
    gfx_get_cursor(&cx, &cy);

    rc_pset_range = gfx_pset(-1, 0, 1, GFX_OP_PSET);
    rc_point_range = gfx_point(info.width, 0, &dummy);
    rc_color_range = gfx_pset(1, 1, 99, GFX_OP_PSET);
    rc_setcur_range = gfx_set_cursor(info.width, 0);
    rc_getcur_null = gfx_get_cursor(0, 0);
    rc_point_null = gfx_point(0, 0, 0);

    to_text();

    FC_CHECK_EQ(rc_pset, GFX_OK);
    FC_CHECK_EQ(v_ps, 5);
    FC_CHECK_EQ(v_pr, 0);
    FC_CHECK_EQ(v_or, 6);
    FC_CHECK_EQ(v_and, 3);
    FC_CHECK_EQ(v_xor, 7);
    FC_CHECK_EQ(v_def, 1);
    FC_CHECK_EQ(cx, 208);
    FC_CHECK_EQ(cy, 100);
    FC_CHECK_EQ(rc_pset_range, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_point_range, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_color_range, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_setcur_range, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_getcur_null, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_point_null, GFX_ERR_INVALID);
    FC_OBSERVE("GRAPHICS2: isolated colored dots, no stray pixels");
}

static void test_multicolor(void) {
    int a = -1, b = -1, c = -1;

    gfx_screen(GFX_MODE_MULTICOLOR, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_clear(GFX_COLOR_DEFAULT);

    gfx_pset(0, 0, 5, GFX_OP_PSET);
    gfx_point(0, 0, &a);

    gfx_preset(0, 0, GFX_COLOR_DEFAULT, GFX_OP_PRESET);
    gfx_point(0, 0, &b);

    gfx_pset(4, 4, 6, GFX_OP_PSET);
    gfx_pset(4, 4, 1, GFX_OP_XOR);
    gfx_point(4, 4, &c);

    to_text();

    FC_CHECK_EQ(a, 5);
    FC_CHECK_EQ(b, 0);
    FC_CHECK_EQ(c, 7);
}

static void test_graphics4(void) {
    int a = -1, b = -1, c = -1;

    gfx_screen(GFX_MODE_GRAPHICS4, GFX_SPRITE_8X8, 0);
    gfx_color(1, 0, 0);
    gfx_clear(GFX_COLOR_DEFAULT);

    /* Adjacent pixels share a byte; each nibble is independent. */
    gfx_pset(10, 10, 5, GFX_OP_PSET);
    gfx_pset(11, 10, 7, GFX_OP_PSET);
    gfx_point(10, 10, &a);
    gfx_point(11, 10, &b);

    gfx_pset(12, 10, 6, GFX_OP_PSET);
    gfx_pset(12, 10, 1, GFX_OP_XOR);
    gfx_point(12, 10, &c);

    to_text();

    FC_CHECK_EQ(a, 5);
    FC_CHECK_EQ(b, 7);
    FC_CHECK_EQ(c, 7);
}

int main(char* args) {
    struct GfxInformation info;
    int vdp;
    int wrong_pset;
    int wrong_point;
    int probe = 0;
    int result;

    (void)args;

    to_text();
    term_puts("GFXPIXELS test\n");

    FC_CHECK_EQ(gfx_get_info(0), GFX_ERR_INVALID);
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    vdp = info.vdp_type;

    /* Text mode has no pixel plane. */
    wrong_pset = gfx_pset(0, 0, 1, GFX_OP_PSET);
    wrong_point = gfx_point(0, 0, &probe);

    test_graphics2();
    test_multicolor();
    if (vdp == VDP_9938 || vdp == VDP_9958) {
        test_graphics4();
    } else {
        FC_SKIP("GRAPHICS4 requires V9938/V9958");
    }

    FC_CHECK_EQ(wrong_pset, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(wrong_point, GFX_ERR_WRONG_MODE);

    to_text();
    FC_OBSERVE("text mode restored after pixel tests");
    result = fc_summary();
    /* ForceCommand restores the screen when this program returns, so hold the
       summary on screen until the user has read it. */
    FC_WAIT("press a key to return to ForceCommand");
    return result;
}
