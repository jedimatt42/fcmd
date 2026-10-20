#include "fctest.h"

/*
 * I31 - palette (AUTO + OBSERVE, F18A only).
 *
 * Covers gfx_palette_set. On non-F18A VDPs every call returns
 * GFX_ERR_UNSUPPORTED; the palette-entry checks and the visual swatches run
 * only on the F18A.
 *
 * On the F18A the test is interactive: in TEXT40 it explains that six
 * swatches will first show the default palette, then the new colors after a
 * key, and a final key returns to TEXT40.
 */

static void to_text(void) {
    gfx_screen(GFX_MODE_TEXT40, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    term_cls();
}

static void announce(const char* what) {
    term_puts("[OBSERVE] ");
    term_puts(what);
    term_putc('\n');
    FC_WAIT("press a key to draw");
}

static void draw_swatches(void) {
    int i;
    for (i = 0; i < 6; i++) {
        int x = 20 + i * 36;
        gfx_line(x, 40, x + 24, 80, 8 + i,
                 GFX_LINE_BOX | GFX_LINE_FILL, GFX_OP_PSET);
    }
}

int main(char* args) {
    struct GfxInformation info;
    int vdp;
    int rc8, rc9, rc10, rc11, rc12, rc13, rc63;
    int rc_idx_neg, rc_idx_hi, rc_r, rc_g, rc_b;
    int result;

    (void)args;

    to_text();
    term_puts("GFXPAL test\n");
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    vdp = info.vdp_type;

    if (vdp != VDP_F18A) {
        FC_CHECK_EQ(gfx_palette_set(0, 0, 0, 0), GFX_ERR_UNSUPPORTED);
        FC_CHECK_EQ(gfx_palette_set(8, 15, 0, 0), GFX_ERR_UNSUPPORTED);
        FC_CHECK_EQ(gfx_palette_set(-1, 16, 16, 16), GFX_ERR_UNSUPPORTED);
        FC_SKIP("palette entry changes require F18A");
        result = fc_summary();
        FC_WAIT("press a key to return to ForceCommand");
        return result;
    }

    announce("F18A palette: six swatches show the default colors; after the "
             "next key they change to the new palette colors");

    gfx_screen(GFX_MODE_GRAPHICS2, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    gfx_clear(GFX_COLOR_DEFAULT);
    draw_swatches();

    /* First look: default palette. */
    FC_WAIT("press a key to apply the new palette colors");

    rc8 = gfx_palette_set(8, 15, 0, 0);    /* red */
    rc9 = gfx_palette_set(9, 0, 15, 0);    /* green */
    rc10 = gfx_palette_set(10, 0, 0, 15);  /* blue */
    rc11 = gfx_palette_set(11, 15, 15, 0); /* yellow */
    rc12 = gfx_palette_set(12, 0, 15, 15); /* cyan */
    rc13 = gfx_palette_set(13, 15, 0, 15); /* magenta */
    rc63 = gfx_palette_set(63, 0, 0, 0);

    rc_idx_neg = gfx_palette_set(-1, 0, 0, 0);
    rc_idx_hi = gfx_palette_set(64, 0, 0, 0);
    rc_r = gfx_palette_set(8, 16, 0, 0);
    rc_g = gfx_palette_set(8, 0, 16, 0);
    rc_b = gfx_palette_set(8, 0, 0, -1);

    /* Second look: new palette. */
    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc8, GFX_OK);
    FC_CHECK_EQ(rc9, GFX_OK);
    FC_CHECK_EQ(rc10, GFX_OK);
    FC_CHECK_EQ(rc11, GFX_OK);
    FC_CHECK_EQ(rc12, GFX_OK);
    FC_CHECK_EQ(rc13, GFX_OK);
    FC_CHECK_EQ(rc63, GFX_OK);
    FC_CHECK_EQ(rc_idx_neg, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_idx_hi, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_r, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_g, GFX_ERR_RANGE);
    FC_CHECK_EQ(rc_b, GFX_ERR_RANGE);

    to_text();
    term_puts("[OBSERVE] swatches changed color after the palette update\n");
    result = fc_summary();
    FC_WAIT("press a key to return to ForceCommand");
    return result;
}
