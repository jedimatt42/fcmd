#include "fctest.h"

/*
 * I29 - pages and copy (AUTO + OBSERVE).
 *
 * Covers gfx_set_page and gfx_copy (cross-page and overlapping same-page),
 * plus unsupported/error paths.
 *
 * Each mode is shown interactively: in TEXT40 the test says what to look for
 * and waits for a key; then it enters the mode and draws; a second key returns
 * to TEXT40, where the AUTO checks for that mode are printed.
 *
 * Modes:
 *   GRAPHICS2   - always available; set_page/copy are unsupported
 *   GRAPHICS4   - V9938/V9958 only; page 1 copied to page 0, overlapping copy
 *   GRAPHICS5   - V9938/V9958 only; page independence
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
    int rc_page, rc_copy;
    struct GfxCopy cp;

    announce("GRAPHICS2", "page/copy are not supported in this mode");

    gfx_screen(GFX_MODE_GRAPHICS2, GFX_SPRITE_8X8, 0);
    rc_page = gfx_set_page(0, 0);
    cp.source_page = 0;
    cp.destination_page = 0;
    cp.x1 = 0;
    cp.y1 = 0;
    cp.x2 = 10;
    cp.y2 = 10;
    cp.destination_x = 20;
    cp.destination_y = 20;
    cp.op = GFX_OP_PSET;
    rc_copy = gfx_copy(&cp);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_page, GFX_ERR_UNSUPPORTED);
    FC_CHECK_EQ(rc_copy, GFX_ERR_UNSUPPORTED);
}

static void test_graphics4(void) {
    struct GfxInformation info;
    struct GfxCopy cp;
    int rc_page1, rc_copy, rc_copy_overlap, rc_copy2;
    int rc_null, rc_range, rc_badpage, rc_diff, rc_sp_range, rc_sp_ok;
    int cp_tl, cp_br, cp_src;
    int ov_100, ov_105, ov_110, ov_115;
    int x;

    announce("GRAPHICS4",
             "a box drawn on page 1 copied onto page 0, then an overlapping "
             "copy of a colored line");

    gfx_screen(GFX_MODE_GRAPHICS4, GFX_SPRITE_8X8, 0);
    gfx_get_info(&info);

    /* Page 0 cleared; page 1 holds a solid box. */
    gfx_set_page(0, 0);
    gfx_clear(0);
    rc_page1 = gfx_set_page(1, 1);
    gfx_clear(0);
    gfx_line(10, 10, 30, 30, 5, GFX_LINE_BOX | GFX_LINE_FILL, GFX_OP_PSET);

    /* Cross-page copy page 1 -> page 0. */
    cp.source_page = 1;
    cp.destination_page = 0;
    cp.x1 = 10;
    cp.y1 = 10;
    cp.x2 = 30;
    cp.y2 = 30;
    cp.destination_x = 50;
    cp.destination_y = 50;
    cp.op = GFX_OP_PSET;
    rc_copy = gfx_copy(&cp);

    gfx_set_page(0, 0);
    cp_tl = sample(50, 50);
    cp_br = sample(70, 70);
    cp_src = sample(10, 10);

    /* Overlapping same-page copy: a gradient line copied 5 px right. */
    gfx_clear(0);
    for (x = 100; x <= 110; x++) {
        gfx_pset(x, 100, (x - 100) + 1, GFX_OP_PSET);
    }
    cp.source_page = 0;
    cp.destination_page = 0;
    cp.x1 = 100;
    cp.y1 = 100;
    cp.x2 = 110;
    cp.y2 = 100;
    cp.destination_x = 105;
    cp.destination_y = 100;
    cp.op = GFX_OP_PSET;
    rc_copy_overlap = gfx_copy(&cp);

    ov_100 = sample(100, 100);
    ov_105 = sample(105, 100);
    ov_110 = sample(110, 100);
    ov_115 = sample(115, 100);

    /* Error paths. */
    rc_null = gfx_copy(0);
    cp.x2 = info.width;
    cp.destination_x = 0;
    cp.destination_y = 0;
    cp.x1 = 0;
    cp.y1 = 0;
    cp.y2 = 10;
    rc_copy2 = gfx_copy(&cp);
    cp.x2 = 10;
    cp.source_page = 2;
    rc_badpage = gfx_copy(&cp);
    cp.source_page = 0;
    cp.destination_x = 250;   /* 250 + (10 - 0) >= width */
    rc_range = gfx_copy(&cp);
    rc_diff = gfx_set_page(0, 1);
    rc_sp_range = gfx_set_page(2, 0);
    rc_sp_ok = gfx_set_page(0, 0);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_page1, GFX_OK);
    FC_CHECK_EQ(rc_copy, GFX_OK);
    FC_CHECK_EQ(cp_tl, 5);
    FC_CHECK_EQ(cp_br, 5);
    FC_CHECK_EQ(cp_src, 0);
    FC_CHECK_EQ(rc_copy_overlap, GFX_OK);
    FC_CHECK_EQ(ov_100, 1);
    FC_CHECK_EQ(ov_105, 1);
    FC_CHECK_EQ(ov_110, 6);
    FC_CHECK_EQ(ov_115, 11);
    FC_CHECK_EQ(rc_null, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_copy2, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_badpage, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_range, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_diff, GFX_ERR_UNSUPPORTED);
    FC_CHECK_EQ(rc_sp_range, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_sp_ok, GFX_OK);
}

static void test_graphics5(void) {
    int rc1, rc0;
    int p1, p0;

    announce("GRAPHICS5", "page 1 holds a box; page 0 is empty");

    gfx_screen(GFX_MODE_GRAPHICS5, GFX_SPRITE_8X8, 0);
    gfx_set_page(0, 0);
    gfx_clear(0);
    rc1 = gfx_set_page(1, 1);
    gfx_clear(0);
    gfx_line(10, 10, 20, 20, 2, GFX_LINE_BOX | GFX_LINE_FILL, GFX_OP_PSET);
    p1 = sample(15, 15);
    rc0 = gfx_set_page(0, 0);
    p0 = sample(15, 15);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc1, GFX_OK);
    FC_CHECK_EQ(p1, 2);
    FC_CHECK_EQ(rc0, GFX_OK);
    FC_CHECK_EQ(p0, 0);
}

int main(char* args) {
    struct GfxInformation info;
    int vdp;
    int text_page;
    int result;

    (void)args;

    to_text();
    term_puts("GFXPAGES test\n");
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    vdp = info.vdp_type;

    /* Text mode has no pages/pixel plane. */
    text_page = gfx_set_page(0, 0);

    test_graphics2();
    if (vdp == VDP_9938 || vdp == VDP_9958) {
        test_graphics4();
        test_graphics5();
    } else {
        FC_SKIP("GRAPHICS4/GRAPHICS5 require V9938/V9958");
    }

    FC_CHECK_EQ(text_page, GFX_ERR_UNSUPPORTED);

    to_text();
    term_puts("[OBSERVE] pages and copies drawn as described\n");
    result = fc_summary();
    FC_WAIT("press a key to return to ForceCommand");
    return result;
}
