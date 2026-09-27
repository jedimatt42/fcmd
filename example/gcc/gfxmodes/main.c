#include "fctest.h"
#include <ioports.h>

/*
 * I24 - graphics modes and capability reporting (interactive).
 *
 * Covers: gfx_screen, gfx_get_info, gfx_color, gfx_clear.
 *
 * The test starts in the safe 40-column text mode, reports which VDP was
 * detected and which modes it supports, then walks each mode one at a time:
 * it announces the next mode, waits for a key, enters the mode, draws something
 * representative, and returns to TEXT40 before checking and reporting.
 *
 * The console keyboard scan (term_kscan) is only ever called while in a text
 * mode; while a graphics mode is active the demonstration is shown for a fixed
 * number of frames instead. The console KSCAN touches VDP registers and the
 * sound-list buffers, which is not safe mid-graphics-mode.
 */

static const int modes[] = {
    GFX_MODE_GRAPHICS1,
    GFX_MODE_TEXT40,
    GFX_MODE_GRAPHICS2,
    GFX_MODE_MULTICOLOR,
    GFX_MODE_GRAPHICS3,
    GFX_MODE_GRAPHICS4,
    GFX_MODE_GRAPHICS5,
    GFX_MODE_GRAPHICS6,
    GFX_MODE_GRAPHICS7,
    GFX_MODE_TEXT80,
    GFX_MODE_YJK_YAE,
    GFX_MODE_YJK_RGB,
    GFX_MODE_YJK,
    GFX_MODE_F18A_TEXT80X30
};

#define NMODES ((int)(sizeof(modes) / sizeof(modes[0])))

/* Return to the safe text mode and reset text colors so results are readable. */
static void to_text40(void) {
    gfx_screen(GFX_MODE_TEXT40, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    term_cls();
}

/* Hold a graphics-mode picture without calling the console keyboard scan. */
static void wait_frames(int frames) {
    while (frames-- > 0) {
        while (!(VDPST & 0x80)) {
            /* wait for the VDP interrupt flag */
        }
    }
}

static int is_yjk(int mode) {
    return mode == GFX_MODE_YJK_YAE || mode == GFX_MODE_YJK_RGB ||
           mode == GFX_MODE_YJK;
}

static int is_text(int mode) {
    return mode == GFX_MODE_TEXT40 || mode == GFX_MODE_TEXT80 ||
           mode == GFX_MODE_F18A_TEXT80X30;
}

static int expected_supported(int mode, int vdp) {
    if (mode == GFX_MODE_GRAPHICS1 || mode == GFX_MODE_TEXT40 ||
        mode == GFX_MODE_GRAPHICS2 || mode == GFX_MODE_MULTICOLOR) {
        return vdp == VDP_9918 || vdp == VDP_9938 || vdp == VDP_9958 ||
               vdp == VDP_F18A;
    }
    if (mode == GFX_MODE_TEXT80) {
        return vdp == VDP_9938 || vdp == VDP_9958 || vdp == VDP_F18A;
    }
    if (mode >= GFX_MODE_GRAPHICS3 && mode <= GFX_MODE_GRAPHICS7) {
        return vdp == VDP_9938 || vdp == VDP_9958;
    }
    if (is_yjk(mode)) {
        return vdp == VDP_9958;
    }
    return mode == GFX_MODE_F18A_TEXT80X30 && vdp == VDP_F18A;
}

static int expected_caps(int mode) {
    switch (mode) {
    case GFX_MODE_GRAPHICS1:
        return GFX_CAP_TILES;
    case GFX_MODE_TEXT40:
    case GFX_MODE_TEXT80:
        return GFX_CAP_TEXT;
    case GFX_MODE_GRAPHICS2:
    case GFX_MODE_MULTICOLOR:
    case GFX_MODE_GRAPHICS3:
        return GFX_CAP_PIXELS | GFX_CAP_LINES | GFX_CAP_CIRCLES | GFX_CAP_PAINT;
    case GFX_MODE_GRAPHICS4:
    case GFX_MODE_GRAPHICS5:
        return GFX_CAP_PIXELS | GFX_CAP_LINES | GFX_CAP_CIRCLES |
               GFX_CAP_PAINT | GFX_CAP_COPY | GFX_CAP_PAGES;
    case GFX_MODE_GRAPHICS6:
    case GFX_MODE_GRAPHICS7:
        return GFX_CAP_PIXELS | GFX_CAP_LINES | GFX_CAP_CIRCLES |
               GFX_CAP_PAINT | GFX_CAP_COPY;
    case GFX_MODE_YJK_YAE:
    case GFX_MODE_YJK_RGB:
    case GFX_MODE_YJK:
        return GFX_CAP_PIXELS | GFX_CAP_LINES | GFX_CAP_CIRCLES |
               GFX_CAP_PAINT | GFX_CAP_COPY | GFX_CAP_YJK;
    case GFX_MODE_F18A_TEXT80X30:
        return GFX_CAP_TEXT | GFX_CAP_ATTRIBUTES | GFX_CAP_TILES;
    default:
        return 0;
    }
}

static const char* mode_name(int mode) {
    switch (mode) {
    case GFX_MODE_GRAPHICS1:      return "GRAPHICS1";
    case GFX_MODE_TEXT40:         return "TEXT40";
    case GFX_MODE_GRAPHICS2:      return "GRAPHICS2";
    case GFX_MODE_MULTICOLOR:     return "MULTICOLOR";
    case GFX_MODE_GRAPHICS3:      return "GRAPHICS3";
    case GFX_MODE_GRAPHICS4:      return "GRAPHICS4";
    case GFX_MODE_GRAPHICS5:      return "GRAPHICS5";
    case GFX_MODE_GRAPHICS6:      return "GRAPHICS6";
    case GFX_MODE_GRAPHICS7:      return "GRAPHICS7";
    case GFX_MODE_TEXT80:         return "TEXT80";
    case GFX_MODE_YJK_YAE:        return "YJK_YAE";
    case GFX_MODE_YJK_RGB:        return "YJK_RGB";
    case GFX_MODE_YJK:            return "YJK";
    case GFX_MODE_F18A_TEXT80X30: return "F18A_TEXT80X30";
    default:                      return "?";
    }
}

static int sprite_for(int mode) {
    return mode == GFX_MODE_F18A_TEXT80X30 ? GFX_SPRITE_16X16
                                           : GFX_SPRITE_8X8;
}

static int pick_color(int mode, struct GfxInformation* info) {
    if (is_yjk(mode)) {
        return GFX_RGB(15, 6, 0);
    }
    if (info->colors >= 16) {
        return COLOR_LTYELLOW;
    }
    if (info->colors >= 4) {
        return 2;
    }
    if (info->colors > 1) {
        return info->colors - 1;
    }
    return 0;
}

static void show_demo(int mode, struct GfxInformation* info) {
    int color = pick_color(mode, info);

    if (is_text(mode)) {
        term_puts("This is ");
        term_puts(mode_name(mode));
        term_puts(" mode.\n");
        term_puts("Pack my box with five dozen liquor jugs.\n");
        return;
    }

    if (mode == GFX_MODE_GRAPHICS1) {
        static const unsigned char tile[8] = {
            0xff, 0x81, 0xbd, 0xa5, 0xa5, 0xbd, 0x81, 0xff
        };
        int i;
        gfx_color(COLOR_CYAN, COLOR_BLACK, COLOR_BLACK);
        gfx_pattern_define(1, tile, 8);
        for (i = 0; i < 6; i++) {
            gfx_tile(13 + i, 10, 1, COLOR_CYAN);
            gfx_tile(13 + i, 11, 1, COLOR_LTYELLOW);
        }
        return;
    }

    if (info->capabilities & GFX_CAP_PIXELS) {
        int r;
        if (!is_yjk(mode)) {
            gfx_clear(GFX_COLOR_DEFAULT);
        }
        /* Border box and a centered circle in the same color, so the Graphics
           II color-group constraint cannot make the shapes ambiguous. */
        gfx_line(2, 2, info->width - 3, info->height - 3,
                 color, GFX_LINE_BOX, GFX_OP_PSET);
        r = (info->width < info->height ? info->width : info->height) / 4;
        gfx_circle(info->width / 2, info->height / 2, r,
                   color, GFX_OP_PSET);
        /* An ellipse in the opposite corner shows the rx/ry form. */
        gfx_ellipse(info->width * 3 / 4, info->height * 3 / 4, r, r / 2,
                    COLOR_CYAN, GFX_OP_PSET);
        /* A filled rectangle in a corner shows a second color and fill. */
        gfx_line(info->width / 16, info->height / 16,
                 info->width * 3 / 8, info->height / 4,
                 COLOR_CYAN, GFX_LINE_BOX | GFX_LINE_FILL, GFX_OP_PSET);
    }
}

static void run_mode(int mode, int vdp, int supported) {
    struct GfxInformation mi;
    int caps = 0;
    int got_mode = -1;
    int color_ok = 0;
    int clear_result = 0;
    int r;

    /* in TEXT40 here */
    term_puts("Next: ");
    term_puts(mode_name(mode));
    term_puts(supported ? " (supported)\n" : " (unsupported on this VDP)\n");
    if (is_text(mode)) {
        term_puts("Press a key to enter, then a key again to return.\n");
    } else {
        term_puts("Press a key to enter; it will return after a short look.\n");
    }
    FC_WAIT("press a key to enter");

    r = gfx_screen(mode, sprite_for(mode), 0);

    if (supported && r == GFX_OK) {
        gfx_get_info(&mi);
        got_mode = mi.mode;
        caps = mi.capabilities;
        color_ok = (gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK) == GFX_OK);
        clear_result = gfx_clear(GFX_COLOR_DEFAULT);
        show_demo(mode, &mi);
        if (is_text(mode)) {
            term_puts("[OBSERVE] press a key to return to TEXT40\n");
            FC_WAIT("press a key");
        } else {
            /* console KSCAN is not safe while a graphics mode is active */
            wait_frames(180);
        }
    }

    /* back to the safe text mode before printing anything else */
    to_text40();

    if (supported) {
        FC_CHECK_EQ(r, GFX_OK);
        if (r == GFX_OK) {
            FC_CHECK_EQ(got_mode, mode);
            FC_CHECK(caps & expected_caps(mode));
            FC_CHECK(color_ok);
            if (is_yjk(mode)) {
                FC_CHECK_EQ(clear_result, GFX_ERR_UNSUPPORTED);
            } else {
                FC_CHECK_EQ(clear_result, GFX_OK);
            }
        }
    } else {
        FC_CHECK_EQ(r, GFX_ERR_UNSUPPORTED);
    }
}

static void announce_availability(int vdp) {
    int i;

    term_puts("VDP type: 0x");
    term_puts(hex_from_uint((unsigned int)vdp));
    term_putc('\n');
    FC_CHECK(vdp == VDP_9918 || vdp == VDP_9938 || vdp == VDP_9958 ||
             vdp == VDP_F18A);

    term_puts("Mode availability:\n");
    for (i = 0; i < NMODES; i++) {
        term_puts("  ");
        term_puts(mode_name(modes[i]));
        term_puts(expected_supported(modes[i], vdp) ? " supported\n"
                                                    : " unsupported\n");
    }
}

static void test_errors(int vdp) {
    struct GfxInformation info;

    FC_CHECK_EQ(gfx_color(16, COLOR_BLACK, COLOR_BLACK), GFX_ERR_RANGE);
    FC_CHECK_EQ(gfx_color(COLOR_WHITE, COLOR_BLACK, 16), GFX_ERR_RANGE);
    FC_CHECK_EQ(gfx_color(GFX_COLOR_DEFAULT, GFX_COLOR_DEFAULT, COLOR_BLACK),
                GFX_OK);
    FC_CHECK_EQ(gfx_clear(99), GFX_ERR_RANGE);

    FC_CHECK_EQ(gfx_screen(GFX_MODE_GRAPHICS2, 0x7f, 0), GFX_ERR_INVALID);
    FC_CHECK_EQ(gfx_screen(99, GFX_SPRITE_8X8, 0), GFX_ERR_UNSUPPORTED);
    FC_CHECK_EQ(gfx_screen(GFX_MODE_GRAPHICS2, GFX_SPRITE_8X8, 0x8000),
                GFX_ERR_UNSUPPORTED);

    if (vdp == VDP_F18A) {
        FC_CHECK_EQ(gfx_screen(GFX_MODE_F18A_TEXT80X30, GFX_SPRITE_8X8, 0),
                    GFX_ERR_UNSUPPORTED);
    }

    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
}

int main(char* args) {
    struct GfxInformation info;
    int vdp;
    int i;

    (void)args;

    /* start in the common-safe mode before anything else */
    to_text40();
    term_puts("GFXMODES test\n");

    FC_CHECK_EQ(gfx_get_info(0), GFX_ERR_INVALID);
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    vdp = info.vdp_type;

    announce_availability(vdp);
    FC_WAIT("press a key to begin the mode walk");

    for (i = 0; i < NMODES; i++) {
        run_mode(modes[i], vdp, expected_supported(modes[i], vdp));
    }

    test_errors(vdp);
    FC_OBSERVE("the display should be back in 40-column text mode");

    return fc_summary();
}
