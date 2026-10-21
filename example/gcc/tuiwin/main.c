#include "fctest.h"

/*
 * I34 - TUI windows (AUTO + OBSERVE).
 *
 * Covers tui_win_open, tui_win_close, tui_win_move, tui_win_resize,
 * tui_win_set_title, tui_win_set_border, tui_win_set_colors, tui_win_gotoxy,
 * tui_win_putc, tui_win_puts, tui_win_printf, tui_win_scroll, and the pool
 * out-of-memory path.
 *
 * Each mode is shown interactively: announce, wait, draw, wait, then return to
 * TEXT40 for the checks.
 *
 * Modes:
 *   TEXT40          - always available
 *   TEXT80          - V9938/V9958/F18A only
 *   F18A_TEXT80X30  - F18A only; per-character window colors
 */

static unsigned char tui_pool[1024];
static unsigned char tiny_pool[16];

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
    FC_WAIT_BOTTOM("press a key to draw");
}

static void write_lines(tui_win_t* win) {
    tui_win_gotoxy(win, 0, 0);
    tui_win_puts(win, "Line one");
    tui_win_gotoxy(win, 0, 1);
    tui_win_puts(win, "Line two");
    tui_win_gotoxy(win, 0, 2);
    tui_win_puts(win, "Line three");
    tui_win_gotoxy(win, 0, 3);
    tui_win_puts(win, "Line four");
    tui_win_gotoxy(win, 0, 4);
    tui_win_puts(win, "Line five");
}

static void test_text40(void) {
    tui_win_t* a;
    tui_win_t* b;
    const char* key = "count";
    int args[2];
    int ok;

    announce("TEXT40",
             "window A moved/resized/re-bordered with scrolled text, then a "
             "second overlapping window B");

    term_cls();
    a = tui_win_open(2, 2, 32, 12);
    ok = (a != 0);
    tui_win_set_title(a, "WINDOW A");
    write_lines(a);

    tui_win_move(a, 3, 3);
    tui_win_resize(a, 34, 12);
    tui_win_set_border(a, TUI_BF_ALL);
    tui_win_set_colors(a, COLOR_WHITE, COLOR_BLACK);
    tui_win_set_title(a, "A2");
    write_lines(a);
    tui_win_scroll(a, 1);

    /* Wraps at the window's right edge (not the screen width). */
    tui_win_gotoxy(a, 0, 5);
    tui_win_puts(a, "a long line that wraps inside the window boundary");

    args[0] = (int)key;
    args[1] = 42;
    tui_win_gotoxy(a, 0, 8);
    tui_win_printf(a, "%s=%d", 2, args);

    tui_win_gotoxy(a, 0, 9);
    tui_win_putc(a, 'X');
    tui_win_puts(a, "YZ");

    b = tui_win_open(20, 8, 16, 8);
    ok = ok && (b != 0);
    tui_win_set_border(b, TUI_BF_ALL);
    tui_win_set_title(b, "B");
    tui_win_gotoxy(b, 0, 0);
    tui_win_puts(b, "in B");

    FC_WAIT_BOTTOM("press a key to close");
    tui_win_close(b);
    tui_win_close(a);
    to_text();

    FC_CHECK(ok);
}

static void test_text80(void) {
    tui_win_t* w;
    int rc;
    int ok;

    announce("TEXT80", "a titled 80-column window with text");

    rc = gfx_screen(GFX_MODE_TEXT80, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);

    w = tui_win_open(4, 2, 50, 10);
    ok = (w != 0);
    tui_win_set_title(w, "WINDOW 80");
    tui_win_gotoxy(w, 0, 0);
    tui_win_puts(w, "80-column window text");

    FC_WAIT_BOTTOM("press a key to close");
    tui_win_close(w);
    to_text();

    FC_CHECK_EQ(rc, GFX_OK);
    FC_CHECK(ok);
}

static void test_f18a_text(void) {
    tui_win_t* w;
    int rc;
    int ok;

    announce("F18A_TEXT80X30", "a window with per-character colors");

    rc = gfx_screen(GFX_MODE_F18A_TEXT80X30, GFX_SPRITE_16X16, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);

    w = tui_win_open(4, 2, 50, 10);
    ok = (w != 0);
    tui_win_set_colors(w, COLOR_LTYELLOW, COLOR_DKBLUE);
    tui_win_set_title(w, "COLOR");
    tui_win_gotoxy(w, 0, 0);
    tui_win_puts(w, "yellow on dark blue");
    tui_win_gotoxy(w, 0, 1);
    tui_win_puts(w, "per-character colors");

    FC_WAIT_BOTTOM("press a key to close");
    tui_win_close(w);
    to_text();

    FC_CHECK_EQ(rc, GFX_OK);
    FC_CHECK(ok);
}

int main(char* args) {
    struct GfxInformation info;
    int vdp;
    int oom;
    int result;

    (void)args;

    to_text();
    term_puts("TUIWIN test\n");
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    vdp = info.vdp_type;

    /* Out-of-memory path: a pool too small for a window returns 0. */
    FC_CHECK_EQ(tui_init(tiny_pool, sizeof(tiny_pool)), 0);
    oom = (tui_win_open(0, 0, 10, 5) != 0);
    FC_CHECK_EQ(tui_init(tui_pool, sizeof(tui_pool)), 0);
    FC_CHECK_EQ(oom, 0);

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
    term_puts("[OBSERVE] TUI windows shown as described\n");
    result = fc_summary();
    FC_WAIT_BOTTOM("press a key to return");
    return result;
}
