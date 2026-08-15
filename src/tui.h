#ifndef _TUI_H
#define _TUI_H 1

#define TUI_EVENT_KEY_NONE 255

enum tui_widget_type {
    TUI_WIDGET_BUTTON,
    TUI_WIDGET_LABEL,
    TUI_WIDGET_LIST,
    TUI_WIDGET_TEXTFIELD,
    TUI_WIDGET_CHECKBOX,
    TUI_WIDGET_PROGRESSBAR,
};

enum tui_event_type {
    TUI_EV_NONE = 0,
    TUI_EV_KEY,
    TUI_EV_MOUSE_CLICK,
    TUI_EV_MOUSE_MOVE,
    TUI_EV_WIDGET_ACTIVATED,
};

struct tui_event {
    enum tui_event_type type;
    struct tui_widget* widget;
    int key;
    int mx;
    int my;
    int buttons;
};

struct tui_widget {
    enum tui_widget_type type;
    int x;
    int y;
    int w;
    int h;
    unsigned char flags;
    struct tui_window* parent;
    struct tui_widget* next;
};

#define TUI_BF_TOP    0x01
#define TUI_BF_BOTTOM 0x02
#define TUI_BF_SIDES  0x04
#define TUI_BF_ALL    0x07

struct tui_window {
    int x;
    int y;
    int w;
    int h;
    int cx;
    int cy;
    int cw;
    int ch;
    int fg;
    int bg;
    unsigned char border_flags;
    char title[41];
    struct tui_widget* children;
    struct tui_widget* focus;
    struct tui_window* next;
};

typedef struct tui_window tui_win_t;
typedef struct tui_widget tui_widget_t;

#ifdef MYBANK

#include "banking.h"

int tui_init(void* pool, int size);
void tui_done(void);
int tui_screen_width(void);
int tui_screen_height(void);
int tui_has_color(void);
void tui_gotoxy(int x, int y);
void tui_putc(int c);
void tui_puts(const char* s);
void tui_set_color(int fg, int bg);
void tui_vdpchar(int pAddr, int ch);
void tui_hline(int x, int y, int w);
void tui_vline(int x, int y, int h);
void tui_box(int x, int y, int w, int h);
void tui_box_title(int x, int y, int w, int h, const char* title);
void tui_fill(int x, int y, int w, int h, int ch);
tui_win_t* tui_win_open(int x, int y, int w, int h);
void tui_win_close(tui_win_t* win);
void tui_win_set_title(tui_win_t* win, const char* title);
void tui_win_move(tui_win_t* win, int x, int y);
void tui_win_resize(tui_win_t* win, int w, int h);
void tui_win_set_border(tui_win_t* win, int flags);
void tui_win_set_colors(tui_win_t* win, int fg, int bg);
void tui_win_gotoxy(tui_win_t* win, int x, int y);
void tui_win_putc(tui_win_t* win, int c);
void tui_win_puts(tui_win_t* win, const char* s);
void tui_win_printf(tui_win_t* win, const char* fmt, ...);
void tui_win_scroll(tui_win_t* win, int lines);
void tui_widget_destroy(tui_widget_t* w);
void tui_widget_show(tui_widget_t* w);
void tui_widget_hide(tui_widget_t* w);
void tui_widget_focus(tui_widget_t* w);
void tui_widget_enable(tui_widget_t* w);
void tui_widget_disable(tui_widget_t* w);
void tui_set_focus(tui_widget_t* w);
tui_widget_t* tui_get_focus(void);
int tui_get_event(struct tui_event* ev);
int tui_dispatch_event(struct tui_event* ev);
tui_widget_t* tui_button_create(tui_win_t* win, int x, int y, int w, const char* label);
void tui_button_set_label(tui_widget_t* btn, const char* label);
tui_widget_t* tui_label_create(tui_win_t* win, int x, int y, const char* text);
void tui_label_set_text(tui_widget_t* lbl, const char* text);
tui_widget_t* tui_list_create(tui_win_t* win, int x, int y, int w, int h);
int tui_list_add(tui_widget_t* list, const char* item);
int tui_list_insert(tui_widget_t* list, int idx, const char* item);
void tui_list_remove(tui_widget_t* list, int idx);
void tui_list_clear(tui_widget_t* list);
int tui_list_count(tui_widget_t* list);
int tui_list_get_selected(tui_widget_t* list);
void tui_list_set_selected(tui_widget_t* list, int idx);
tui_widget_t* tui_textfield_create(tui_win_t* win, int x, int y, int w);
void tui_textfield_set(tui_widget_t* tf, const char* text);
const char* tui_textfield_get(tui_widget_t* tf);
void tui_textfield_set_cursor(tui_widget_t* tf, int pos);
tui_widget_t* tui_checkbox_create(tui_win_t* win, int x, int y, const char* label);
void tui_checkbox_set(tui_widget_t* cb, int state);
int tui_checkbox_get(tui_widget_t* cb);
tui_widget_t* tui_progressbar_create(tui_win_t* win, int x, int y, int w);
void tui_progressbar_set(tui_widget_t* pb, int value);
int tui_message_box(const char* title, const char* msg);
int tui_confirm_box(const char* title, const char* msg);
int tui_input_box(const char* title, const char* prompt, char* buf, int len);
tui_win_t* tui_menu_create(int x, int y, const char** items, int count);
void tui_menu_destroy(tui_win_t* menu);
int tui_menu_get_selected(tui_win_t* menu);
void tui_button_render(tui_widget_t* w);
int tui_button_handle_key(tui_widget_t* w, int key);
void tui_label_render(tui_widget_t* w);
void tui_list_render(tui_widget_t* w);
int tui_list_handle_key(tui_widget_t* w, int key);
void tui_textfield_render(tui_widget_t* w);
int tui_textfield_handle_key(tui_widget_t* w, int key);
void tui_checkbox_render(tui_widget_t* w);
int tui_checkbox_handle_key(tui_widget_t* w, int key);
void tui_progressbar_render(tui_widget_t* w);
void tui_render_widget(tui_widget_t* w);
void tui_menu_render(tui_win_t* menu);
int tui_menu_handle_key(tui_win_t* menu, int key);

DECLARE_BANKED(tui_init, BANK(11), int, bk_tui_init, (void* pool, int size), (pool, size))
DECLARE_BANKED_VOID(tui_done, BANK(11), bk_tui_done, (), ())
DECLARE_BANKED(tui_screen_width, BANK(11), int, bk_tui_screen_width, (), ())
DECLARE_BANKED(tui_screen_height, BANK(11), int, bk_tui_screen_height, (), ())
DECLARE_BANKED(tui_has_color, BANK(11), int, bk_tui_has_color, (), ())
DECLARE_BANKED_VOID(tui_gotoxy, BANK(11), bk_tui_gotoxy, (int x, int y), (x, y))
DECLARE_BANKED_VOID(tui_putc, BANK(11), bk_tui_putc, (int c), (c))
DECLARE_BANKED_VOID(tui_puts, BANK(11), bk_tui_puts, (const char* s), (s))
DECLARE_BANKED_VOID(tui_set_color, BANK(11), bk_tui_set_color, (int fg, int bg), (fg, bg))
DECLARE_BANKED_VOID(tui_vdpchar, BANK(11), bk_tui_vdpchar, (int pAddr, int ch), (pAddr, ch))
DECLARE_BANKED_VOID(tui_hline, BANK(11), bk_tui_hline, (int x, int y, int w), (x, y, w))
DECLARE_BANKED_VOID(tui_vline, BANK(11), bk_tui_vline, (int x, int y, int h), (x, y, h))
DECLARE_BANKED_VOID(tui_box, BANK(11), bk_tui_box, (int x, int y, int w, int h), (x, y, w, h))
DECLARE_BANKED_VOID(tui_box_title, BANK(11), bk_tui_box_title, (int x, int y, int w, int h, const char* title), (x, y, w, h, title))
DECLARE_BANKED_VOID(tui_fill, BANK(11), bk_tui_fill, (int x, int y, int w, int h, int ch), (x, y, w, h, ch))
DECLARE_BANKED(tui_win_open, BANK(11), tui_win_t*, bk_tui_win_open, (int x, int y, int w, int h), (x, y, w, h))
DECLARE_BANKED_VOID(tui_win_close, BANK(11), bk_tui_win_close, (tui_win_t* win), (win))
DECLARE_BANKED_VOID(tui_win_set_title, BANK(11), bk_tui_win_set_title, (tui_win_t* win, const char* title), (win, title))
DECLARE_BANKED_VOID(tui_win_move, BANK(11), bk_tui_win_move, (tui_win_t* win, int x, int y), (win, x, y))
DECLARE_BANKED_VOID(tui_win_resize, BANK(11), bk_tui_win_resize, (tui_win_t* win, int w, int h), (win, w, h))
DECLARE_BANKED_VOID(tui_win_set_border, BANK(11), bk_tui_win_set_border, (tui_win_t* win, int flags), (win, flags))
DECLARE_BANKED_VOID(tui_win_set_colors, BANK(11), bk_tui_win_set_colors, (tui_win_t* win, int fg, int bg), (win, fg, bg))
DECLARE_BANKED_VOID(tui_win_gotoxy, BANK(11), bk_tui_win_gotoxy, (tui_win_t* win, int x, int y), (win, x, y))
DECLARE_BANKED_VOID(tui_win_putc, BANK(11), bk_tui_win_putc, (tui_win_t* win, int c), (win, c))
DECLARE_BANKED_VOID(tui_win_puts, BANK(11), bk_tui_win_puts, (tui_win_t* win, const char* s), (win, s))
DECLARE_BANKED_VOID(tui_win_printf, BANK(11), bk_tui_win_printf, (tui_win_t* win, const char* fmt, ...), (win, fmt))
DECLARE_BANKED_VOID(tui_win_scroll, BANK(11), bk_tui_win_scroll, (tui_win_t* win, int lines), (win, lines))
DECLARE_BANKED_VOID(tui_widget_destroy, BANK(11), bk_tui_widget_destroy, (tui_widget_t* w), (w))
DECLARE_BANKED_VOID(tui_widget_show, BANK(11), bk_tui_widget_show, (tui_widget_t* w), (w))
DECLARE_BANKED_VOID(tui_widget_hide, BANK(11), bk_tui_widget_hide, (tui_widget_t* w), (w))
DECLARE_BANKED_VOID(tui_widget_focus, BANK(11), bk_tui_widget_focus, (tui_widget_t* w), (w))
DECLARE_BANKED_VOID(tui_widget_enable, BANK(11), bk_tui_widget_enable, (tui_widget_t* w), (w))
DECLARE_BANKED_VOID(tui_widget_disable, BANK(11), bk_tui_widget_disable, (tui_widget_t* w), (w))
DECLARE_BANKED_VOID(tui_set_focus, BANK(11), bk_tui_set_focus, (tui_widget_t* w), (w))
DECLARE_BANKED(tui_get_focus, BANK(11), tui_widget_t*, bk_tui_get_focus, (), ())
DECLARE_BANKED(tui_get_event, BANK(11), int, bk_tui_get_event, (struct tui_event* ev), (ev))
DECLARE_BANKED(tui_dispatch_event, BANK(11), int, bk_tui_dispatch_event, (struct tui_event* ev), (ev))
DECLARE_BANKED_VOID(tui_render_widget, BANK(11), bk_tui_render_widget, (tui_widget_t* w), (w))

DECLARE_BANKED(tui_button_create, BANK(12), tui_widget_t*, bk_tui_button_create, (tui_win_t* win, int x, int y, int w, const char* label), (win, x, y, w, label))
DECLARE_BANKED_VOID(tui_button_set_label, BANK(12), bk_tui_button_set_label, (tui_widget_t* btn, const char* label), (btn, label))
DECLARE_BANKED(tui_label_create, BANK(12), tui_widget_t*, bk_tui_label_create, (tui_win_t* win, int x, int y, const char* text), (win, x, y, text))
DECLARE_BANKED_VOID(tui_label_set_text, BANK(12), bk_tui_label_set_text, (tui_widget_t* lbl, const char* text), (lbl, text))
DECLARE_BANKED(tui_list_create, BANK(12), tui_widget_t*, bk_tui_list_create, (tui_win_t* win, int x, int y, int w, int h), (win, x, y, w, h))
DECLARE_BANKED(tui_list_add, BANK(12), int, bk_tui_list_add, (tui_widget_t* list, const char* item), (list, item))
DECLARE_BANKED(tui_list_insert, BANK(12), int, bk_tui_list_insert, (tui_widget_t* list, int idx, const char* item), (list, idx, item))
DECLARE_BANKED_VOID(tui_list_remove, BANK(12), bk_tui_list_remove, (tui_widget_t* list, int idx), (list, idx))
DECLARE_BANKED_VOID(tui_list_clear, BANK(12), bk_tui_list_clear, (tui_widget_t* list), (list))
DECLARE_BANKED(tui_list_count, BANK(12), int, bk_tui_list_count, (tui_widget_t* list), (list))
DECLARE_BANKED(tui_list_get_selected, BANK(12), int, bk_tui_list_get_selected, (tui_widget_t* list), (list))
DECLARE_BANKED_VOID(tui_list_set_selected, BANK(12), bk_tui_list_set_selected, (tui_widget_t* list, int idx), (list, idx))
DECLARE_BANKED(tui_textfield_create, BANK(12), tui_widget_t*, bk_tui_textfield_create, (tui_win_t* win, int x, int y, int w), (win, x, y, w))
DECLARE_BANKED_VOID(tui_textfield_set, BANK(12), bk_tui_textfield_set, (tui_widget_t* tf, const char* text), (tf, text))
DECLARE_BANKED(tui_textfield_get, BANK(12), const char*, bk_tui_textfield_get, (tui_widget_t* tf), (tf))
DECLARE_BANKED_VOID(tui_textfield_set_cursor, BANK(12), bk_tui_textfield_set_cursor, (tui_widget_t* tf, int pos), (tf, pos))
DECLARE_BANKED(tui_checkbox_create, BANK(12), tui_widget_t*, bk_tui_checkbox_create, (tui_win_t* win, int x, int y, const char* label), (win, x, y, label))
DECLARE_BANKED_VOID(tui_checkbox_set, BANK(12), bk_tui_checkbox_set, (tui_widget_t* cb, int state), (cb, state))
DECLARE_BANKED(tui_checkbox_get, BANK(12), int, bk_tui_checkbox_get, (tui_widget_t* cb), (cb))
DECLARE_BANKED(tui_progressbar_create, BANK(12), tui_widget_t*, bk_tui_progressbar_create, (tui_win_t* win, int x, int y, int w), (win, x, y, w))
DECLARE_BANKED_VOID(tui_progressbar_set, BANK(12), bk_tui_progressbar_set, (tui_widget_t* pb, int value), (pb, value))
DECLARE_BANKED(tui_message_box, BANK(12), int, bk_tui_message_box, (const char* title, const char* msg), (title, msg))
DECLARE_BANKED(tui_confirm_box, BANK(12), int, bk_tui_confirm_box, (const char* title, const char* msg), (title, msg))
DECLARE_BANKED(tui_input_box, BANK(12), int, bk_tui_input_box, (const char* title, const char* prompt, char* buf, int len), (title, prompt, buf, len))
DECLARE_BANKED(tui_menu_create, BANK(12), tui_win_t*, bk_tui_menu_create, (int x, int y, const char** items, int count), (x, y, items, count))
DECLARE_BANKED_VOID(tui_menu_destroy, BANK(12), bk_tui_menu_destroy, (tui_win_t* menu), (menu))
DECLARE_BANKED(tui_menu_get_selected, BANK(12), int, bk_tui_menu_get_selected, (tui_win_t* menu), (menu))

DECLARE_BANKED_VOID(tui_button_render, BANK(12), bk_tui_button_render, (tui_widget_t* w), (w))
DECLARE_BANKED(tui_button_handle_key, BANK(12), int, bk_tui_button_handle_key, (tui_widget_t* w, int key), (w, key))
DECLARE_BANKED_VOID(tui_label_render, BANK(12), bk_tui_label_render, (tui_widget_t* w), (w))
DECLARE_BANKED_VOID(tui_list_render, BANK(12), bk_tui_list_render, (tui_widget_t* w), (w))
DECLARE_BANKED(tui_list_handle_key, BANK(12), int, bk_tui_list_handle_key, (tui_widget_t* w, int key), (w, key))
DECLARE_BANKED_VOID(tui_textfield_render, BANK(12), bk_tui_textfield_render, (tui_widget_t* w), (w))
DECLARE_BANKED(tui_textfield_handle_key, BANK(12), int, bk_tui_textfield_handle_key, (tui_widget_t* w, int key), (w, key))
DECLARE_BANKED_VOID(tui_checkbox_render, BANK(12), bk_tui_checkbox_render, (tui_widget_t* w), (w))
DECLARE_BANKED(tui_checkbox_handle_key, BANK(12), int, bk_tui_checkbox_handle_key, (tui_widget_t* w, int key), (w, key))
DECLARE_BANKED_VOID(tui_progressbar_render, BANK(12), bk_tui_progressbar_render, (tui_widget_t* w), (w))
DECLARE_BANKED_VOID(tui_menu_render, BANK(12), bk_tui_menu_render, (tui_win_t* menu), (menu))
DECLARE_BANKED(tui_menu_handle_key, BANK(12), int, bk_tui_menu_handle_key, (tui_win_t* menu, int key), (menu, key))

#endif
#endif
