#include "fctest.h"

/*
 * I30 - tiles and sprites (AUTO + OBSERVE).
 *
 * Covers gfx_pattern_define, gfx_tile, gfx_sprite_pattern, gfx_sprite,
 * gfx_sprite_loc, gfx_sprite_hide, gfx_sprite_enable, gfx_sprite_status,
 * and their error codes.
 *
 * Each mode is shown interactively: in TEXT40 the test says what to look for
 * and waits for a key; then it enters the mode and draws; a second key returns
 * to TEXT40, where the AUTO checks for that mode are printed.
 *
 * Modes:
 *   GRAPHICS1   - always available; tiles + one sprite
 *   GRAPHICS2   - always available; tiles are unsupported, one sprite
 */

static const unsigned char tile_data[8] = {
    0xff, 0x81, 0xbd, 0xa5, 0xa5, 0xbd, 0x81, 0xff
};
static const unsigned char ball_data[8] = {
    0x18, 0x3c, 0x7e, 0xff, 0xff, 0x7e, 0x3c, 0x18
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

static void test_graphics1(void) {
    struct GfxSpriteStatus st;
    int rc_pat, rc_tile, rc_spat, rc_sprite, rc_loc, rc_enable, rc_status;
    int rc_hide, rc_disable;
    int rc_bad_bytes, rc_null_data, rc_pat_range;
    int rc_tile_range, rc_tile_color;
    int rc_spr_num, rc_spr_xy, rc_spr_pat, rc_spr_color, rc_hide_range;
    int i;

    announce("GRAPHICS1", "a row of hollow-square tiles and one red sprite");

    gfx_screen(GFX_MODE_GRAPHICS1, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_CYAN, COLOR_BLACK, COLOR_BLACK);
    gfx_clear(GFX_COLOR_DEFAULT);

    rc_pat = gfx_pattern_define(1, tile_data, 8);
    rc_tile = GFX_OK;
    for (i = 0; i < 6; i++) {
        rc_tile = gfx_tile(10 + i, 10, 1, COLOR_LTYELLOW);
    }
    rc_spat = gfx_sprite_pattern(0, ball_data, 8);
    rc_sprite = gfx_sprite(0, 0, COLOR_LTRED, 120, 80);
    rc_loc = gfx_sprite_loc(0, 150, 90);
    rc_enable = gfx_sprite_enable(1);
    rc_status = gfx_sprite_status(&st);

    rc_bad_bytes = gfx_pattern_define(1, tile_data, 7);
    rc_null_data = gfx_pattern_define(1, 0, 8);
    rc_pat_range = gfx_pattern_define(256, tile_data, 8);
    rc_tile_range = gfx_tile(32, 10, 1, COLOR_CYAN);
    rc_tile_color = gfx_tile(10, 10, 1, 99);
    rc_spr_num = gfx_sprite(32, 0, 1, 10, 10);
    rc_spr_xy = gfx_sprite(0, 0, 1, 256, 10);
    rc_spr_pat = gfx_sprite(0, 256, 1, 10, 10);
    rc_spr_color = gfx_sprite(0, 0, 16, 10, 10);
    rc_hide = gfx_sprite_hide(1);
    rc_hide_range = gfx_sprite_hide(32);

    FC_WAIT("press a key to return to TEXT40");
    rc_disable = gfx_sprite_enable(0);
    to_text();

    FC_CHECK_EQ(rc_pat, GFX_OK);
    FC_CHECK_EQ(rc_tile, GFX_OK);
    FC_CHECK_EQ(rc_spat, GFX_OK);
    FC_CHECK_EQ(rc_sprite, GFX_OK);
    FC_CHECK_EQ(rc_loc, GFX_OK);
    FC_CHECK_EQ(rc_enable, GFX_OK);
    FC_CHECK_EQ(rc_status, GFX_OK);
    FC_CHECK_EQ(rc_hide, GFX_OK);
    FC_CHECK_EQ(rc_disable, GFX_OK);
    FC_CHECK_EQ(rc_bad_bytes, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_null_data, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_pat_range, GFX_ERR_INVALID);
    FC_CHECK_EQ(rc_tile_range, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_tile_color, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_spr_num, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_spr_xy, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_spr_pat, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_spr_color, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_hide_range, GFX_ERR_RANGE);
}

static void test_graphics2(void) {
    int rc_pat, rc_tile, rc_spat, rc_sprite, rc_status;
    struct GfxSpriteStatus st;
    struct GfxInformation info;

    announce("GRAPHICS2",
             "tiles are unsupported in bitmap mode; one red sprite");

    gfx_screen(GFX_MODE_GRAPHICS2, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_CYAN, COLOR_BLACK, COLOR_BLACK);
    gfx_get_info(&info);
    gfx_clear(GFX_COLOR_DEFAULT);

    rc_pat = gfx_pattern_define(1, tile_data, 8);
    rc_tile = gfx_tile(5, 5, 1, COLOR_CYAN);
    rc_spat = gfx_sprite_pattern(0, ball_data, 8);
    rc_sprite = gfx_sprite(0, 0, COLOR_LTRED, 128, 96);
    gfx_sprite_enable(1);
    rc_status = gfx_sprite_status(&st);

    FC_WAIT("press a key to return to TEXT40");
    gfx_sprite_enable(0);
    to_text();

    FC_CHECK_EQ(rc_pat, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(rc_tile, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(rc_spat, GFX_OK);
    FC_CHECK_EQ(rc_sprite, GFX_OK);
    FC_CHECK_EQ(rc_status, GFX_OK);
    /* Regression (F13): the sprite pattern table must not sit on top of the
       Graphics II pattern table (0x0000-0x17FF). */
    FC_CHECK(info.sprite_pattern_addr >= info.pattern_addr + 0x1800);
}

int main(char* args) {
    struct GfxInformation info;
    struct GfxSpriteStatus st;
    int text_pat, text_tile, text_sprite, text_status, text_null_status;
    int result;

    (void)args;

    to_text();
    term_puts("GFXTILES test\n");
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);

    /* Text mode has no tile or sprite plane. */
    text_pat = gfx_pattern_define(0, tile_data, 8);
    text_tile = gfx_tile(0, 0, 0, 0);
    text_sprite = gfx_sprite(0, 0, 0, 0, 0);
    text_status = gfx_sprite_status(&st);
    text_null_status = gfx_sprite_status(0);

    test_graphics1();
    test_graphics2();

    FC_CHECK_EQ(text_pat, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(text_tile, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(text_sprite, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(text_status, GFX_ERR_WRONG_MODE);
    FC_CHECK_EQ(text_null_status, GFX_ERR_INVALID);

    to_text();
    term_puts("[OBSERVE] tiles and sprites drawn as described\n");
    result = fc_summary();
    FC_WAIT("press a key to return to ForceCommand");
    return result;
}
