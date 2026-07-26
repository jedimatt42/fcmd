#include "banks.h"
#define MYBANK BANK(12)

#include "tui_internal.h"
#include "conio.h"
#include "editor.h"

int tui_message_box(const char* title, const char* msg) {
    if (!title) title = "Message";
    if (!msg) msg = "";

    int msg_len = tui_strlen(msg);
    int win_w = msg_len + 4;
    if (win_w < 20) win_w = 20;
    if (win_w > displayWidth - 4) win_w = displayWidth - 4;
    int win_h = 7;
    int win_x = (displayWidth - win_w) / 2;
    int win_y = (displayHeight - win_h) / 2;

    tui_win_t* win = bk_tui_win_open(win_x, win_y, win_w, win_h);
    if (!win) return 0;
    bk_tui_win_set_title(win, title);

    int label_x = (win_w - 2 - msg_len) / 2;
    if (label_x < 1) label_x = 1;
    bk_tui_label_create(win, label_x, 1, msg);

    int btn_w = 6;
    int btn_x = (win_w - 2 - btn_w) / 2;
    tui_widget_t* ok = bk_tui_button_create(win, btn_x, 3, btn_w, "  OK  ");
    bk_tui_widget_focus(ok);

    int result = 0;
    while (1) {
        struct tui_event ev;
        if (bk_tui_get_event(&ev)) {
            if (ev.type == TUI_EV_WIDGET_ACTIVATED && ev.widget == ok) {
                result = 0;
                break;
            }
            if (ev.type == TUI_EV_KEY && (ev.key == KEY_BREAK || ev.key == 2)) {
                break;
            }
        }
    }

    bk_tui_win_close(win);
    return result;
}

int tui_confirm_box(const char* title, const char* msg) {
    if (!title) title = "Confirm";
    if (!msg) msg = "";

    int msg_len = tui_strlen(msg);
    int win_w = msg_len + 4;
    if (win_w < 24) win_w = 24;
    if (win_w > displayWidth - 4) win_w = displayWidth - 4;
    int win_h = 7;
    int win_x = (displayWidth - win_w) / 2;
    int win_y = (displayHeight - win_h) / 2;

    tui_win_t* win = bk_tui_win_open(win_x, win_y, win_w, win_h);
    if (!win) return 0;
    bk_tui_win_set_title(win, title);

    int label_x = (win_w - 2 - msg_len) / 2;
    if (label_x < 1) label_x = 1;
    bk_tui_label_create(win, label_x, 1, msg);

    int btn_w = 6;
    int total_btns = btn_w * 2 + 2;
    int btn_start = (win_w - 2 - total_btns) / 2;
    tui_widget_t* yes = bk_tui_button_create(win, btn_start, 3, btn_w, "  Yes ");
    tui_widget_t* no  = bk_tui_button_create(win, btn_start + btn_w + 2, 3, btn_w, "  No  ");
    bk_tui_widget_focus(yes);

    int result = 0;
    while (1) {
        struct tui_event ev;
        if (bk_tui_get_event(&ev)) {
            if (ev.type == TUI_EV_WIDGET_ACTIVATED) {
                if (ev.widget == yes) { result = 1; break; }
                if (ev.widget == no)  { result = 0; break; }
            }
            if (ev.type == TUI_EV_KEY) {
                if (ev.key == KEY_BREAK || ev.key == 2) { result = 0; break; }
            }
        }
    }

    bk_tui_win_close(win);
    return result;
}

int tui_input_box(const char* title, const char* prompt, char* buf, int len) {
    if (!title) title = "Input";
    if (!prompt) prompt = "";
    if (!buf || len <= 0) return 0;

    int prompt_len = tui_strlen(prompt);
    int win_w = prompt_len + 6;
    if (win_w < 30) win_w = 30;
    if (win_w > displayWidth - 4) win_w = displayWidth - 4;
    if (len + 2 < win_w) win_w = len + 4;
    int win_h = 7;
    int win_x = (displayWidth - win_w) / 2;
    int win_y = (displayHeight - win_h) / 2;

    tui_win_t* win = bk_tui_win_open(win_x, win_y, win_w, win_h);
    if (!win) return 0;
    bk_tui_win_set_title(win, title);

    bk_tui_label_create(win, 1, 1, prompt);

    int field_w = win_w - 4;
    tui_widget_t* tf = bk_tui_textfield_create(win, 1, 2, field_w);
    bk_tui_textfield_set(tf, buf);
    bk_tui_widget_focus(tf);

    int btn_w = 6;
    int btn_x = (win_w - 2 - btn_w) / 2;
    tui_widget_t* ok = bk_tui_button_create(win, btn_x, 4, btn_w, "  OK  ");
    bk_tui_widget_focus(ok);
    bk_tui_widget_focus(tf);

    int result = 0;
    while (1) {
        struct tui_event ev;
        if (bk_tui_get_event(&ev)) {
            if (ev.type == TUI_EV_WIDGET_ACTIVATED) {
                if (ev.widget == ok) {
                    const char* text = bk_tui_textfield_get(tf);
                    tui_strncpy(buf, text, len);
                    result = 1;
                    break;
                }
            }
            if (ev.type == TUI_EV_KEY) {
                if (ev.key == KEY_BREAK || ev.key == 2) { result = 0; break; }
                if (ev.key == 13) {
                    const char* text = bk_tui_textfield_get(tf);
                    tui_strncpy(buf, text, len);
                    result = 1;
                    break;
                }
            }
        }
    }

    bk_tui_win_close(win);
    return result;
}
