#ifndef _TUI_INTERNAL_H
#define _TUI_INTERNAL_H 1

#include "tui.h"
#include "globals.h"
#include "vdp.h"
#include "conio.h"

#define TUI_WF_VISIBLE  0x01
#define TUI_WF_ENABLED  0x02
#define TUI_WF_FOCUSED  0x04

struct tui_state {
    struct tui_window* windows;
    struct tui_widget* focus;
    struct tui_widget* widget_under_mouse;
    unsigned char* pool_start;
    unsigned char* pool_end;
    unsigned char* pool_cur;
};

struct tui_button_data {
    char label[41];
};

struct tui_label_data {
    char text[81];
};

struct tui_list_item {
    struct tui_list_item* next;
    char text[40];
};

#define TUI_LIST_MAX_ITEMS 200

struct tui_list_data {
    int count;
    int selected;
    int scroll_offset;
    struct tui_list_item* head;
    struct tui_list_item* tail;
};

struct tui_textfield_data {
    char buffer[81];
    int cursor;
    int len;
};

struct tui_checkbox_data {
    char label[41];
    int state;
};

struct tui_progressbar_data {
    int value;
};

extern struct tui_state* tui_ctx;

extern void* memcpy(void* dest, const void* src, int cnt);

static inline int tui_strlen(const char* s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

static inline void tui_strncpy(char* dst, const char* src, int max) {
    int i = 0;
    while (i < max - 1 && src && src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

static inline void* tui_alloc(int size) {
    size = (size + 1) & ~1;
    if (tui_ctx->pool_cur + size > tui_ctx->pool_end)
        return 0;
    void* ptr = tui_ctx->pool_cur;
    tui_ctx->pool_cur += size;
    return ptr;
}

static inline void* tui_widget_data(struct tui_widget* w) {
    return (unsigned char*)w + sizeof(struct tui_widget);
}

struct tui_widget* tui_focus_next();
struct tui_widget* tui_focus_prev();

void tui_render_widget(struct tui_widget* w);
int  tui_handle_widget_key(struct tui_widget* w, int key);

#endif
