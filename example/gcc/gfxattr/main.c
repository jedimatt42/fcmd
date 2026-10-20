#include "fctest.h"

/*
 * I32 - text attributes and tile semantics (COND + AUTO + OBSERVE).
 *
 * Covers gfx_tile / gfx_pattern_define in the text/attribute modes:
 *   - MULTICOLOR: tile functions are wrong-mode.
 *   - TEXT80:     tiles/attributes are not part of the mode.
 *   - F18A_TEXT80X30 (F18A only): per-character color attributes via gfx_tile.
 *
 * Each mode is shown interactively: in TEXT40 the test says what to look for
 * and waits for a key; then it enters the mode and draws; a second key returns
 * to TEXT40, where the AUTO checks for that mode are printed.
 */

static const unsigned char tile_data[8] = {
    0xff, 0x81, 0xbd, 0xa5, 0xa5, 0xbd, 0x81, 0xff
};

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

static void test_multicolor(void) {
    int rc_tile, rc_pat;

    announce("MULTICOLOR", "tile functions are unsupported in this mode");

    gfx_screen(GFX_MODE_MULTICOLOR, GFX_SPRITE_8X8, 0);
    rc_tile = gfx_tile(5, 5, 1, 1);
    rc_pat = gfx_pattern_define(1, tile_data, 8);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_tile, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(rc_pat, GFX_ERR_WRONG_MODE);
}

static void test_text80(void) {
    struct GfxInformation info;
    int rc_screen, rc_tile, rc_pat;

    announce("TEXT80",
             "80-column text; tile and per-character attribute functions are "
             "not part of this mode");

    rc_screen = gfx_screen(GFX_MODE_TEXT80, GFX_SPRITE_8X8, 0);
    gfx_get_info(&info);
    term_puts("TEXT80: 80 columns, no per-character attributes.\n");
    rc_tile = gfx_tile(5, 5, 65, 3);
    rc_pat = gfx_pattern_define(65, tile_data, 8);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_screen, GFX_OK);
    FC_CHECK_EQ(info.width, 80);
    FC_CHECK(info.capabilities & GFX_CAP_TEXT);
    FC_CHECK(!(info.capabilities & GFX_CAP_ATTRIBUTES));
    FC_CHECK(!(info.capabilities & GFX_CAP_TILES));
    FC_CHECK_EQ(rc_tile, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(rc_pat, GFX_ERR_WRONG_MODE);
}

static void test_f18a_text(void) {
    struct GfxInformation info;
    int rc_screen, rc_tile;
    int rc_range_x, rc_range_y, rc_range_pat, rc_range_color;
    int i;

    announce("F18A_TEXT80X30",
             "the letters A-F on one row, each in a different color "
             "(per-character attributes)");

    rc_screen = gfx_screen(GFX_MODE_F18A_TEXT80X30, GFX_SPRITE_16X16, 0);
    gfx_get_info(&info);

    rc_tile = GFX_OK;
    for (i = 0; i < 6; i++) {
        rc_tile = gfx_tile(10 + i, 10, 'A' + i, 2 + i);
    }

    rc_range_x = gfx_tile(80, 10, 'A', 3);
    rc_range_y = gfx_tile(10, 30, 'A', 3);
    rc_range_pat = gfx_tile(10, 10, 256, 3);
    rc_range_color = gfx_tile(10, 10, 'A', 16);

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc_screen, GFX_OK);
    FC_CHECK_EQ(info.width, 80);
    FC_CHECK_EQ(info.height, 30);
    FC_CHECK(info.capabilities & GFX_CAP_TEXT);
    FC_CHECK(info.capabilities & GFX_CAP_TILES);
    FC_CHECK(info.capabilities & GFX_CAP_ATTRIBUTES);
    FC_CHECK_EQ(rc_tile, GFX_OK);
    FC_CHECK_EQ(rc_range_x, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_range_y, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_range_pat, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_range_color, GFX_ERR_RANGE);
}

int main(char* args) {
    struct GfxInformation info;
    int vdp;
    int text_tile, text_pat;
    int result;

    (void)args;

    to_text();
    term_puts("GFXATTR test\n");
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    vdp = info.vdp_type;

    /* TEXT40 has no tile plane. */
    text_tile = gfx_tile(0, 0, 0, 1);
    text_pat = gfx_pattern_define(0, tile_data, 8);

    test_multicolor();
    if (vdp == VDP_9938 || vdp == VDP_9958 || vdp == VDP_F18A) {
        test_text80();
    } else {
        FC_SKIP("TEXT80 requires V9938/V9958/F18A");
    }
    if (vdp == VDP_F18A) {
        test_f18a_text();
    } else {
        FC_SKIP("F18A_TEXT80X30 requires F18A");
    }

    FC_CHECK_EQ(text_tile, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(text_pat, GFX_ERR_WRONG_MODE);

    to_text();
    term_puts("[OBSERVE] attribute/tile behavior shown as described\n");
    result = fc_summary();
    FC_WAIT("press a key to return to ForceCommand");
    return result;
}
