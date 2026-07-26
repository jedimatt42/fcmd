#include "banks.h"
#define MYBANK BANK(12)

#include "tui_internal.h"
#include "conio.h"

tui_win_t* tui_menu_create(int x, int y, const char** items, int count) {
    if (!items || count <= 0) return 0;
    int max_w = 0;
    for (int i = 0; i < count; i++) {
        int len = tui_strlen(items[i] ? items[i] : "");
        if (len > max_w) max_w = len;
    }
    int win_w = max_w + 4;
    int win_h = count + 2;
    if (win_h > displayHeight - y) {
        win_h = displayHeight - y;
    }

    tui_win_t* win = bk_tui_win_open(x, y, win_w, win_h);
    if (!win) return 0;

    tui_widget_t* list = bk_tui_list_create(win, 1, 1, win_w - 2, win_h - 2);
    for (int i = 0; i < count; i++) {
        bk_tui_list_add(list, items[i] ? items[i] : "");
    }
    if (count > 0) {
        bk_tui_list_set_selected(list, 0);
    }
    bk_tui_widget_focus(list);
    return win;
}

void tui_menu_destroy(tui_win_t* menu) {
    if (!menu) return;
    bk_tui_win_close(menu);
}

int tui_menu_get_selected(tui_win_t* menu) {
    if (!menu || !menu->children) return -1;
    tui_widget_t* w = menu->children;
    while (w) {
        if (w->type == TUI_WIDGET_LIST) {
            return bk_tui_list_get_selected(w);
        }
        w = w->next;
    }
    return -1;
}

void tui_menu_render(tui_win_t* menu) {
    if (!menu || !menu->children) return;
    tui_widget_t* w = menu->children;
    while (w) {
        if (w->type == TUI_WIDGET_LIST) {
            bk_tui_list_render(w);
            return;
        }
        w = w->next;
    }
}

int tui_menu_handle_key(tui_win_t* menu, int key) {
    if (!menu || !menu->focus) return 0;
    return bk_tui_list_handle_key(menu->focus, key);
}
