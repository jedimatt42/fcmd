#include "fctest.h"

/*
 * I36 - TUI events and focus (AUTO + OBSERVE).
 *
 * Covers tui_get_event, tui_dispatch_event, tui_set_focus/tui_get_focus,
 * widget show/hide/enable/disable/destroy, the focus ring (FCTN-8), and the
 * BREAK close-window path.
 *
 * The interactive part opens an empty window behind a window with widgets.
 * FCTN-8 moves focus (buttons and checkbox; brackets change), BREAK closes the empty window, and Q
 * ends the event loop.
 */

static unsigned char tui_pool[1024];

#define KEY_Q 81
#define KEY_BREAK 2
#define KEY_NEXT 6

static void to_text(void) {
    gfx_screen(GFX_MODE_TEXT40, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    term_cls();
}

int main(char* args) {
    struct GfxInformation info;
    struct tui_event ev;
    tui_win_t* empty;
    tui_win_t* win;
    tui_widget_t* btn1;
    tui_widget_t* btn2;
    tui_widget_t* btn3;
    tui_widget_t* cb;
    int d_null, d_none, d_key, g_null;
    int f1, f2, f3, f5;
    int ok;
    int events = 0;
    int activated = 0;
    int result;

    (void)args;

    to_text();
    term_puts("TUIEVENTS test\n");
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    (void)info;

    FC_CHECK_EQ(tui_init(tui_pool, sizeof(tui_pool)), 0);

    /* dispatch_event / get_event argument checks. */
    d_null = tui_dispatch_event(0);
    ev.type = TUI_EV_NONE;
    d_none = tui_dispatch_event(&ev);
    ev.type = TUI_EV_KEY;
    ev.key = 'A';
    d_key = tui_dispatch_event(&ev);
    g_null = tui_get_event(0);

    /* An empty window first, so it is the oldest (tail) for BREAK. */
    empty = tui_win_open(20, 10, 16, 6);
    tui_win_set_title(empty, "EMPTY");
    win = tui_win_open(1, 1, 34, 10);
    tui_win_set_title(win, "EVENTS");
    ok = (empty != 0 && win != 0);

    btn1 = tui_button_create(win, 2, 1, 10, "One");
    btn2 = tui_button_create(win, 2, 3, 10, "Two");
    cb = tui_checkbox_create(win, 2, 5, "Opt");

    /* Focus / enable / disable / hide / show / destroy. */
    tui_set_focus(btn1);
    f1 = (tui_get_focus() == btn1);
    tui_set_focus(btn2);
    f2 = (tui_get_focus() == btn2);
    tui_widget_disable(btn2);
    f3 = (tui_get_focus() == 0);
    tui_widget_enable(btn2);
    tui_widget_hide(btn2);
    tui_widget_show(btn2);
    tui_set_focus(btn1);
    tui_widget_destroy(btn1);
    f5 = (tui_get_focus() == 0);

    /* Add a focusable button; keep the checkbox so it is in the ring too. */
    btn3 = tui_button_create(win, 2, 1, 10, "One");
    tui_set_focus(btn3);

    tui_gotoxy(0, 18);
    term_puts("FCTN-8 next; BREAK close; Q quit\n");
    tui_gotoxy(0, 19);
    term_puts("ENTER activates focus\n");

    /* Event loop: FCTN-8 moves focus, BREAK closes the empty window, Q exits. */
    for (;;) {
        if (tui_get_event(&ev)) {
            events++;
            tui_dispatch_event(&ev);
            if (ev.type == TUI_EV_WIDGET_ACTIVATED) activated++;
            if (ev.key == KEY_Q) break;
        }
    }

    tui_win_close(win);
    tui_win_close(empty);
    to_text();

    FC_CHECK_EQ(d_null, 0);
    FC_CHECK_EQ(d_none, 0);
    FC_CHECK_EQ(d_key, 1);
    FC_CHECK_EQ(g_null, 0);
    FC_CHECK(ok);
    FC_CHECK(btn3 != 0);
    FC_CHECK(f1);
    FC_CHECK(f2);
    FC_CHECK(f3);
    FC_CHECK(f5);
    FC_CHECK(events >= 1);
    (void)activated;
    (void)KEY_BREAK;
    (void)KEY_NEXT;

    tui_done();

    to_text();
    term_puts("[OBSERVE] focus ring moved and EMPTY window closed by BREAK\n");
    result = fc_summary();
    FC_WAIT_BOTTOM("press a key to return");
    return result;
}
