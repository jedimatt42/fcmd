#include "fctest.h"

/*
 * I37 - TUI dialogs and menus (AUTO + OBSERVE).
 *
 * Covers tui_message_box, tui_confirm_box, tui_input_box, tui_menu_create,
 * tui_menu_destroy, and tui_menu_get_selected.
 *
 * The message/confirm/input dialogs are modal and require user input. The menu
 * checks are automatic (create, selection, navigation, destroy).
 */

static unsigned char tui_pool[2048];

static const char* menu_items[3] = { "One", "Two", "Three" };

static void to_text(void) {
    struct GfxInformation info;
    if (gfx_get_info(&info) == GFX_OK && info.vdp_type == VDP_F18A) {
        /* 80x30 is the only mode that supports the TIPI mouse pointer, so run
           the dialogs there when the hardware allows it. */
        gfx_screen(GFX_MODE_F18A_TEXT80X30, GFX_SPRITE_16X16, 0);
    } else {
        gfx_screen(GFX_MODE_TEXT40, GFX_SPRITE_8X8, 0);
    }
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_BLACK);
    term_cls();
}

static void set_hello(char* b) {
    b[0] = 'h'; b[1] = 'e'; b[2] = 'l'; b[3] = 'l'; b[4] = 'o'; b[5] = 0;
}

int main(char* args) {
    tui_win_t* menu;
    struct tui_event ev;
    int m_null, m_zero, m_ok;
    int sel0, menu_sel, nullsel;
    int rc_msg, rc_confirm, rc_input;
    char buf[32];
    int result;

    (void)args;

    to_text();
    term_puts("TUIDIALOGS test\n");
    FC_CHECK_EQ(tui_init(tui_pool, sizeof(tui_pool)), 0);

    /* ---- menu (automatic) ---- */
    m_null = (tui_menu_create(0, 0, 0, 3) == 0);
    m_zero = (tui_menu_create(0, 0, menu_items, 0) == 0);
    menu = tui_menu_create(2, 2, menu_items, 3);
    m_ok = (menu != 0);
    sel0 = tui_menu_get_selected(menu);
    nullsel = tui_menu_get_selected(0);

    tui_gotoxy(0, 20);
    term_puts("Menu: UP/DOWN select; Q to continue\n");
    tui_mouse_enable();
    for (;;) {
        if (tui_get_event(&ev)) {
            if (ev.type == TUI_EV_KEY && ev.key == 'Q') break;
        }
    }
    tui_mouse_disable();
    menu_sel = tui_menu_get_selected(menu);

    tui_menu_destroy(menu);
    tui_menu_destroy(0);

    /* ---- message box ---- */
    tui_init(tui_pool, sizeof(tui_pool));
    to_text();
    term_puts("Message box: press ENTER on OK (or BREAK)\n");
    rc_msg = tui_message_box("Message", "Press ENTER on OK");

    /* ---- confirm box ---- */
    tui_init(tui_pool, sizeof(tui_pool));
    to_text();
    term_puts("Confirm: ENTER=Yes, FCTN-8 then ENTER=No, BREAK=No\n");
    term_puts("Return is 1 for Yes, 0 for No/BREAK.\n");
    rc_confirm = tui_confirm_box("Confirm", "Choose Yes or No");

    /* ---- input box ---- */
    tui_init(tui_pool, sizeof(tui_pool));
    to_text();
    term_puts("Input: edit the text then ENTER, or BREAK to cancel\n");
    set_hello(buf);
    rc_input = tui_input_box("Input", "Text:", buf, (int)sizeof(buf));

    to_text();
    FC_CHECK_EQ(m_null, 1);
    FC_CHECK_EQ(m_zero, 1);
    FC_CHECK(m_ok);
    FC_CHECK_EQ(sel0, 0);
    FC_CHECK(menu_sel >= 0 && menu_sel <= 2);
    FC_CHECK_EQ(nullsel, -1);
    FC_CHECK_EQ(rc_msg, 0);
    FC_CHECK(rc_confirm == 0 || rc_confirm == 1);
    FC_CHECK(rc_input == 0 || rc_input == 1);

    term_puts("input buffer: ");
    term_puts(buf);
    term_putc('\n');

    tui_done();

    to_text();
    term_puts("[OBSERVE] dialogs shown and menu navigated as described\n");
    result = fc_summary();
    FC_WAIT_BOTTOM("press a key to return");
    return result;
}
