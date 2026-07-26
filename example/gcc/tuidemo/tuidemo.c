#include <fc_api.h>

static char tui_pool[4096];

static void demo_widgets() {
    int w = tui_screen_width();
    int h = tui_screen_height();

    tui_win_t* win = tui_win_open(4, 3, 50, 16);
    tui_win_set_title(win, " Widget Demo ");

    tui_widget_t* list = tui_list_create(win, 1, 1, 28, 5);
    tui_list_add(list, "Apples");
    tui_list_add(list, "Bananas");
    tui_list_add(list, "Cherries");
    tui_list_add(list, "Dates");
    tui_list_set_selected(list, 0);

    tui_widget_t* cb_log = tui_checkbox_create(win, 1, 7, "Logging");
    tui_widget_t* cb_opt = tui_checkbox_create(win, 1, 8, "Verbose");
    tui_checkbox_set(cb_log, 1);

    tui_widget_t* tf = tui_textfield_create(win, 1, 10, 46);
    tui_textfield_set(tf, "Hello TI-99/4A!");

    tui_widget_t* pb = tui_progressbar_create(win, 1, 12, 46);

    tui_widget_t* ok = tui_button_create(win, 15, 14, 10, "   OK   ");
    tui_widget_focus(list);

    int running = 1;
    int prog = 0;
    struct tui_event ev;

    while (running) {
        prog = (prog + 1) % 101;
        tui_progressbar_set(pb, prog);

        if (tui_get_event(&ev)) {
            if (ev.type == TUI_EV_WIDGET_ACTIVATED && ev.widget == ok) {
                running = 0;
            }
            if (ev.type == TUI_EV_KEY && ev.key == 2) {
                running = 0;
            }
        }
    }

    tui_win_close(win);
}

static int demo_message() {
    return tui_message_box(" Notice ", "This is a message box.\nIt displays information.");
}

static int demo_confirm() {
    return tui_confirm_box(" Question ", "Do you like the TUI library?");
}

static int demo_input() {
    char buf[80];
    buf[0] = 0;
    int ok = tui_input_box(" Input ", "Enter your name:", buf, 80);
    if (ok) {
        char msg[100];
        msg[0] = 0;
        tui_message_box(" Hello ", buf);
    }
    return ok;
}

static void demo_startup() {
    int w = tui_screen_width();
    int h = tui_screen_height();
    int c = tui_has_color();

    tui_box(0, 0, w, 3);
    tui_gotoxy(1, 1);
    tui_puts("TUI Demo");
    tui_gotoxy(w - 12, 1);
    tui_puts("v1.0");
    tui_gotoxy(2, 2);
    tui_puts("Screen: ");
    tui_putc('0' + w / 10);
    tui_putc('0' + w % 10);
    tui_putc('x');
    tui_putc('0' + h / 10);
    tui_putc('0' + h % 10);
    if (c) {
        tui_puts("  Color: yes");
    } else {
        tui_puts("  Color: no");
    }
}

int main(char* args) {
    (void)args;

    term_cls();
    if (tui_init(tui_pool, sizeof(tui_pool)) != 0) return 1;

    demo_startup();

    tui_win_t* menu = tui_win_open(10, 5, 40, 14);
    tui_win_set_title(menu, " Main Menu ");

    tui_widget_t* demo_list = tui_list_create(menu, 1, 1, 28, 6);
    tui_list_add(demo_list, "1. Widget Controls");
    tui_list_add(demo_list, "2. Message Box");
    tui_list_add(demo_list, "3. Confirm Dialog");
    tui_list_add(demo_list, "4. Input Dialog");
    tui_list_add(demo_list, "5. Quit");
    tui_list_set_selected(demo_list, 0);

    tui_widget_t* run_btn = tui_button_create(menu, 30, 1, 8, "  Run  ");
    tui_widget_t* quit_btn = tui_button_create(menu, 30, 3, 8, " Quit ");

    tui_widget_t* auto_cb = tui_checkbox_create(menu, 1, 8, "Run demo after selection");
    tui_checkbox_set(auto_cb, 1);

    tui_widget_focus(demo_list);

    struct tui_event ev;
    int running = 1;

    while (running) {
        if (tui_get_event(&ev)) {
            if (ev.type == TUI_EV_WIDGET_ACTIVATED) {
                int sel = tui_list_get_selected(demo_list);
                if (ev.widget == quit_btn) {
                    running = 0;
                } else if (ev.widget == run_btn || ev.widget == demo_list) {
                    switch (sel) {
                        case 0: demo_widgets(); break;
                        case 1: demo_message(); break;
                        case 2: demo_confirm(); break;
                        case 3: demo_input(); break;
                        case 4: running = 0; break;
                    }
                    tui_box(0, 0, tui_screen_width(), 3);
                    tui_gotoxy(1, 1);
                    tui_puts("TUI Demo  v1.0");
                    if (!tui_checkbox_get(auto_cb)) {
                        tui_widget_focus(demo_list);
                    } else if (sel + 1 < 5) {
                        tui_list_set_selected(demo_list, sel + 1);
                    }
                }
            }
            if (ev.type == TUI_EV_KEY && ev.key == 2) {
                running = 0;
            }
        }
    }

    tui_win_close(menu);
    tui_done();
    term_cls();
    term_puts("TUI Demo finished.\n");
    return 0;
}
