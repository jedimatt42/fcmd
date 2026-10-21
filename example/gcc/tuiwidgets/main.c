#include "fctest.h"

/*
 * I35 - TUI widgets (AUTO + OBSERVE).
 *
 * Covers button/label/list/textfield/checkbox/progressbar creation, their
 * accessors, and the list index/count behavior, plus the widget allocation
 * out-of-memory path.
 *
 * Each mode is shown interactively: announce, wait, draw, wait, then return to
 * TEXT40 for the checks.
 */

static unsigned char tui_pool[2048];
static unsigned char tiny_pool[128];

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

static void test_text40(void) {
    tui_win_t* win;
    tui_widget_t* btn;
    tui_widget_t* lbl;
    tui_widget_t* list;
    tui_widget_t* tf;
    tui_widget_t* cb;
    tui_widget_t* pb;
    const char* tf_text;
    int add0, add1, add2, ins1, ins_bad;
    int count3, sel_set, count_rm, sel_rm, count_clr, sel_clr;
    int add_bad, tf_ok, cb1, cb0, cb5;
    int ok;

    announce("TEXT40",
             "a button, label, list (3 items), text field, checkbox, and "
             "progress bar inside one window");

    /* The widget pool has no free; reset it so each mode starts clean. */
    tui_init(tui_pool, sizeof(tui_pool));
    term_cls();
    win = tui_win_open(1, 1, 38, 20);
    ok = (win != 0);
    tui_win_set_title(win, "WIDGETS");

    btn = tui_button_create(win, 2, 1, 10, "OK");
    tui_button_set_label(btn, "Go");
    lbl = tui_label_create(win, 2, 3, "Name:");
    tui_label_set_text(lbl, "Name");

    list = tui_list_create(win, 2, 5, 16, 5);
    add0 = tui_list_add(list, "alpha");
    add1 = tui_list_add(list, "beta");
    add2 = tui_list_add(list, "gamma");
    count3 = tui_list_count(list);
    ins1 = tui_list_insert(list, 1, "inserted");
    tui_list_set_selected(list, 2);
    sel_set = tui_list_get_selected(list);
    tui_list_remove(list, 2);
    count_rm = tui_list_count(list);
    sel_rm = tui_list_get_selected(list);
    tui_list_clear(list);
    count_clr = tui_list_count(list);
    sel_clr = tui_list_get_selected(list);
    add_bad = tui_list_add(lbl, "nope");   /* not a list */

    /* Final list contents for the observation. */
    tui_list_add(list, "one");
    tui_list_add(list, "two");
    tui_list_add(list, "three");
    tui_list_set_selected(list, 1);
    ins_bad = tui_list_insert(list, 99, "nope");

    tf = tui_textfield_create(win, 22, 5, 12);
    tui_textfield_set(tf, "hello");
    tf_text = tui_textfield_get(tf);
    tui_textfield_set_cursor(tf, 99);

    cb = tui_checkbox_create(win, 22, 7, "check");
    tui_checkbox_set(cb, 1);
    cb1 = tui_checkbox_get(cb);
    tui_checkbox_set(cb, 0);
    cb0 = tui_checkbox_get(cb);
    tui_checkbox_set(cb, 5);
    cb5 = tui_checkbox_get(cb);
    tui_checkbox_set(cb, 1);

    pb = tui_progressbar_create(win, 22, 9, 12);
    tui_progressbar_set(pb, 75);

    tf_ok = (tf_text != 0 && tf_text[0] == 'h' && tf_text[1] == 'e' &&
             tf_text[2] == 'l' && tf_text[3] == 'l' && tf_text[4] == 'o' &&
             tf_text[5] == 0);

    FC_WAIT_BOTTOM("press a key to close");
    tui_win_close(win);
    to_text();

    FC_CHECK(ok);
    FC_CHECK(btn != 0);
    FC_CHECK(lbl != 0);
    FC_CHECK(list != 0);
    FC_CHECK(tf != 0);
    FC_CHECK(cb != 0);
    FC_CHECK(pb != 0);
    FC_CHECK_EQ(add0, 0);
    FC_CHECK_EQ(add1, 1);
    FC_CHECK_EQ(add2, 2);
    FC_CHECK_EQ(count3, 3);
    FC_CHECK_EQ(ins1, 1);
    FC_CHECK_EQ(sel_set, 2);
    FC_CHECK_EQ(count_rm, 3);
    FC_CHECK_EQ(sel_rm, -1);
    FC_CHECK_EQ(count_clr, 0);
    FC_CHECK_EQ(sel_clr, -1);
    FC_CHECK_EQ(add_bad, -1);
    FC_CHECK_EQ(ins_bad, -1);
    FC_CHECK(tf_ok);
    FC_CHECK_EQ(cb1, 1);
    FC_CHECK_EQ(cb0, 0);
    FC_CHECK_EQ(cb5, 1);
}

static void test_f18a_text(void) {
    tui_win_t* win;
    tui_widget_t* lbl;
    tui_widget_t* btn;
    tui_widget_t* cb;
    tui_widget_t* pb;
    int rc;
    int ok;

    announce("F18A_TEXT80X30", "a colored window containing widgets");

    rc = gfx_screen(GFX_MODE_F18A_TEXT80X30, GFX_SPRITE_16X16, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    /* Reset the no-free widget pool for this mode. */
    tui_init(tui_pool, sizeof(tui_pool));
    term_cls();

    win = tui_win_open(4, 2, 50, 12);
    ok = (win != 0);
    tui_win_set_colors(win, COLOR_LTYELLOW, COLOR_DKBLUE);
    tui_win_set_title(win, "WIDGETS");

    lbl = tui_label_create(win, 2, 1, "Color label");
    btn = tui_button_create(win, 2, 3, 12, "Press");
    cb = tui_checkbox_create(win, 2, 5, "Check");
    tui_checkbox_set(cb, 1);
    pb = tui_progressbar_create(win, 2, 7, 20);
    tui_progressbar_set(pb, 60);

    FC_WAIT_BOTTOM("press a key to close");
    tui_win_close(win);
    to_text();

    FC_CHECK_EQ(rc, GFX_OK);
    FC_CHECK(ok);
    FC_CHECK(lbl != 0);
    FC_CHECK(btn != 0);
    FC_CHECK(cb != 0);
    FC_CHECK(pb != 0);
}

int main(char* args) {
    struct GfxInformation info;
    int vdp;
    int tiny_size;
    int oom;
    int result;

    (void)args;

    to_text();
    term_puts("TUIWIDGETS test\n");
    FC_CHECK_EQ(gfx_get_info(&info), GFX_OK);
    vdp = info.vdp_type;

    /* Widget OOM: a pool with room for the state+window but not a widget. */
    tiny_size = (int)sizeof(struct tui_window) + 24;
    if (tiny_size > (int)sizeof(tiny_pool)) tiny_size = sizeof(tiny_pool);
    FC_CHECK_EQ(tui_init(tiny_pool, tiny_size), 0);
    {
        tui_win_t* w = tui_win_open(2, 2, 20, 8);
        oom = (w != 0) ? (tui_button_create(w, 1, 1, 8, "x") == 0) : 0;
        if (w) tui_win_close(w);
    }
    FC_CHECK_EQ(tui_init(tui_pool, sizeof(tui_pool)), 0);
    FC_CHECK_EQ(oom, 1);

    test_text40();
    if (vdp == VDP_F18A) {
        test_f18a_text();
    } else {
        FC_SKIP("F18A_TEXT80X30 requires F18A");
    }

    tui_done();

    to_text();
    term_puts("[OBSERVE] TUI widgets shown as described\n");
    result = fc_summary();
    FC_WAIT_BOTTOM("press a key to return");
    return result;
}
