#include <fc_api.h>
#include <ioports.h>
#include <kscan.h>

/*
 * Minimal reproduction / sequence probe for graphics mode transitions.
 *
 * The header is not screen-safe, so ForceCommand restores a text mode when this
 * program returns. The argument is a list of modes to walk, each followed by a
 * return to TEXT40 before the next. Flags may be mixed in:
 *
 *   n   do not draw anything in the graphics modes
 *   r   do not return to TEXT40 explicitly at the end (let ForceCommand do it)
 *   w   wait ~180 VDP frames in each graphics mode (mimics GFXMODES)
 *   k   after returning to TEXT40, wait for a key (console KSCAN, like GFXMODES)
 *   s   simple drawing (two diagonals only); default matches GFXMODES show_demo
 *
 * Mode letters:
 *   t   TEXT40        m   MULTICOLOR
 *   1   GRAPHICS1     2   GRAPHICS2
 *   3   GRAPHICS3     4   GRAPHICS4     5   GRAPHICS5
 *   6   GRAPHICS6     7   GRAPHICS7
 *
 * Examples:
 *   GFXBITMAP           GRAPHICS2 only (default)
 *   GFXBITMAP 12        GRAPHICS1 then GRAPHICS2
 *   GFXBITMAP 12m       GRAPHICS1, GRAPHICS2, MULTICOLOR
 *   GFXBITMAP n 12m     same without drawing
 *
 * No terminal output happens while a graphics mode is active.
 */

static int mode_for(char c) {
    switch (c) {
    case 't': return GFX_MODE_TEXT40;
    case '1': return GFX_MODE_GRAPHICS1;
    case '2': return GFX_MODE_GRAPHICS2;
    case '3': return GFX_MODE_GRAPHICS3;
    case '4': return GFX_MODE_GRAPHICS4;
    case '5': return GFX_MODE_GRAPHICS5;
    case '6': return GFX_MODE_GRAPHICS6;
    case '7': return GFX_MODE_GRAPHICS7;
    case '8': return GFX_MODE_TEXT80;
    case 'm': return GFX_MODE_MULTICOLOR;
    case 'y': return GFX_MODE_YJK;
    case 'f': return GFX_MODE_F18A_TEXT80X30;
    default:  return -1;
    }
}

static int sprite_for(int mode) {
    return mode == GFX_MODE_F18A_TEXT80X30 ? GFX_SPRITE_16X16
                                           : GFX_SPRITE_8X8;
}

static void wait_vblanks(int frames) {
    while (frames-- > 0) {
        while (!(VDPST & 0x80)) {
            /* wait for the VDP interrupt flag */
        }
    }
}

/* A console keyboard-scan wait, in text mode (mimics fctest FC_WAIT). */
static void wait_key(void) {
    term_puts("[KEY] press a key\n");
    while ((KSCAN_STATUS & KSCAN_MASK) == 0) {
        term_kscan(5);
    }
    term_kscan(5);
}

static int simple_draw = 0;
static int draw_box = 1;
static int draw_circle = 1;

static void draw_mode(int mode) {
    struct GfxInformation info;

    if (mode == GFX_MODE_TEXT40) {
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
        }
        return;
    }

    gfx_get_info(&info);
    if (!(info.capabilities & GFX_CAP_PIXELS)) {
        return;
    }
    gfx_clear(GFX_COLOR_DEFAULT);
    if (simple_draw) {
        gfx_line(8, 8, info.width - 9, info.height - 9,
                 COLOR_LTYELLOW, GFX_LINE_NORMAL, GFX_OP_PSET);
        gfx_line(info.width - 9, 8, 8, info.height - 9,
                 COLOR_LTYELLOW, GFX_LINE_NORMAL, GFX_OP_PSET);
        return;
    }
    /* matches gfxmodes show_demo() */
    gfx_line(info.width / 6, info.height / 6,
             info.width * 5 / 6, info.height * 5 / 6,
             COLOR_LTYELLOW, GFX_LINE_NORMAL, GFX_OP_PSET);
    gfx_line(info.width * 5 / 6, info.height / 6,
             info.width / 6, info.height * 5 / 6,
             COLOR_LTYELLOW, GFX_LINE_NORMAL, GFX_OP_PSET);
    if (draw_box) {
        gfx_line(info.width / 8, info.height / 8,
                 info.width * 7 / 8, info.height * 7 / 8,
                 COLOR_CYAN, GFX_LINE_BOX, GFX_OP_PSET);
    }
    if (draw_circle) {
        gfx_circle(info.width / 2, info.height / 2, info.width / 8,
                   COLOR_LTYELLOW, 0, 360, 100, GFX_OP_PSET);
    }
}

int main(char* args) {
    int mode_list[16];
    int count = 0;
    int draw = 1;
    int explicit_text = 1;
    int wait = 0;
    int keywait = 0;
    char* p;
    int i;

    if (args) {
        for (p = args; *p != 0; p++) {
            int mode;
            if (*p == 'n') {
                draw = 0;
                continue;
            }
            if (*p == 'r') {
                explicit_text = 0;
                continue;
            }
            if (*p == 'w') {
                wait = 1;
                continue;
            }
            if (*p == 'k') {
                keywait = 1;
                continue;
            }
            if (*p == 's') {
                simple_draw = 1;
                continue;
            }
            if (*p == 'b') {
                draw_circle = 0;
                continue;
            }
            if (*p == 'o') {
                draw_box = 0;
                continue;
            }
            mode = mode_for(*p);
            if (mode >= 0 && count < 16) {
                mode_list[count++] = mode;
            }
        }
    }
    if (count == 0) {
        mode_list[count++] = GFX_MODE_GRAPHICS2;
    }

    term_puts("GFXBITMAP: walking modes\n");
    for (i = 0; i < count; i++) {
        gfx_screen(mode_list[i], sprite_for(mode_list[i]), 0);
        if (draw) {
            draw_mode(mode_list[i]);
        }
        if (wait && mode_list[i] != GFX_MODE_TEXT40) {
            wait_vblanks(180);
        }
        if (i < count - 1) {
            gfx_screen(GFX_MODE_TEXT40, GFX_SPRITE_8X8, 0);
            if (keywait) {
                wait_key();
            }
        }
    }

    if (explicit_text) {
        gfx_screen(GFX_MODE_TEXT40, GFX_SPRITE_8X8, 0);
        term_cls();
    }
    if (keywait) {
        wait_key();
    }

    return 0;
}
