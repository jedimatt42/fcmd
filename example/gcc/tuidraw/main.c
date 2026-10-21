#include "fctest.h"

/*
 * I33 - TUI drawing primitives (AUTO + OBSERVE).
 *
 * Covers tui_init, tui_done, tui_screen_width, tui_screen_height,
 * tui_has_color, tui_gotoxy, tui_putc, tui_puts, tui_set_color, tui_hline,
 * tui_vline, tui_box, tui_box_title, tui_fill.
 *
 * Each mode is shown interactively: announce what to look for, wait, draw,
 * wait, then return to TEXT40 for the checks.
 *
 * Modes:
 *   TEXT40          - always available
 *   TEXT80          - V9938/V9958/F18A only
 *   F18A_TEXT80X30  - F18A only; per-character colors via tui_set_color
 */

static unsigned char tui_pool[512];

#define POOL_OK 0

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

static void test_text40(void) {
    announce("TEXT40",
             "a text string, single character, horizontal/vertical lines, "
             "a box, a titled box, and a filled area");

    term_cls();
    tui_gotoxy(2, 1);
    tui_puts("TUIDRAW TEXT40");
    tui_gotoxy(2, 3);
    tui_putc('X');
    tui_hline(2, 5, 10);
    tui_vline(2, 6, 5);
    tui_box(4, 7, 20, 6);
    tui_box_title(26, 7, 12, 6, "TITLE");
    tui_fill(4, 15, 10, 3, '*');

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(tui_screen_width(), 40);
    FC_CHECK_EQ(tui_screen_height(), 24);
    FC_CHECK_EQ(tui_has_color(), 0);
}

static void test_text80(void) {
    int rc, w, h, color;

    announce("TEXT80", "a text string, a wide box, a titled box, and a fill");

    rc = gfx_screen(GFX_MODE_TEXT80, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    w = tui_screen_width();
    h = tui_screen_height();
    color = tui_has_color();

    term_cls();
    tui_gotoxy(2, 1);
    tui_puts("TUIDRAW TEXT80");
    tui_box(2, 3, 40, 8);
    tui_box_title(44, 3, 30, 8, "WIDE");
    tui_fill(2, 13, 20, 3, '#');

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc, GFX_OK);
    FC_CHECK_EQ(w, 80);
    FC_CHECK(h >= 24 && h <= 26);
    FC_CHECK_EQ(color, 0);
}

static void test_f18a_text(void) {
    int rc, w, h, color;

    announce("F18A_TEXT80X30",
             "text and a box in different per-character colors");

    rc = gfx_screen(GFX_MODE_F18A_TEXT80X30, GFX_SPRITE_16X16, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    w = tui_screen_width();
    h = tui_screen_height();
    color = tui_has_color();

    term_cls();
    tui_set_color(COLOR_LTRED, COLOR_BLACK);
    tui_gotoxy(2, 1);
    tui_puts("RED TEXT");
    tui_set_color(COLOR_CYAN, COLOR_BLACK);
    tui_gotoxy(2, 2);
    tui_puts("CYAN TEXT");
    tui_set_color(COLOR_LTYELLOW, COLOR_BLACK);
    tui_gotoxy(2, 3);
    tui_puts("YELLOW TEXT");
    tui_set_color(COLOR_LTGREEN, COLOR_BLACK);
    tui_box(2, 5, 36, 8);
    tui_fill(4, 14, 20, 3, '=');

    FC_WAIT("press a key to return to TEXT40");
    to_text();

    FC_CHECK_EQ(rc, GFX_OK);
    FC_CHECK_EQ(w, 80);
    FC_CHECK_EQ(h, 30);
    FC_CHECK_EQ(color, 1);
}

int main(char* args) {
    struct GfxInformation info;
    int vdp;
    int result;

    (void)args;

    to_text();
    term_puts("TUIDRAW test\n");
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    vdp = info.vdp_type;

    FC_CHECK_EQ(tui_init(tui_pool, 1), -1);
    FC_CHECK_EQ(tui_init(tui_pool, sizeof(tui_pool)), POOL_OK);

    test_text40();
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

    tui_done();

    to_text();
    term_puts("[OBSERVE] TUI drawing shown as described\n");
    result = fc_summary();
    FC_WAIT("press a key to return to ForceCommand");
    return result;
}
