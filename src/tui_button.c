#include "banks.h"
#define MYBANK BANK(12)

#include "tui_internal.h"
#include "conio.h"

tui_widget_t* tui_button_create(tui_win_t* win, int x, int y, int w, const char* label) {
    tui_widget_t* wgt = (tui_widget_t*)tui_alloc(sizeof(tui_widget_t) + sizeof(struct tui_button_data));
    if (!wgt) return 0;
    wgt->type = TUI_WIDGET_BUTTON;
    wgt->x = x;
    wgt->y = y;
    wgt->w = w;
    wgt->h = 1;
    wgt->flags = TUI_WF_VISIBLE | TUI_WF_ENABLED;
    wgt->parent = win;
    wgt->next = win->children;
    win->children = wgt;
    struct tui_button_data* d = (struct tui_button_data*)tui_widget_data(wgt);
    tui_strncpy(d->label, label, 41);
    bk_tui_render_widget(wgt);
    return wgt;
}

void tui_button_set_label(tui_widget_t* btn, const char* label) {
    if (!btn || btn->type != TUI_WIDGET_BUTTON) return;
    struct tui_button_data* d = (struct tui_button_data*)tui_widget_data(btn);
    tui_strncpy(d->label, label, 41);
    bk_tui_render_widget(btn);
}

void tui_button_render(tui_widget_t* w) {
    struct tui_button_data* d = (struct tui_button_data*)tui_widget_data(w);
    int sx = w->parent->cx + w->x;
    int sy = w->parent->cy + w->y;
    char lb, rb;
    if (!(w->flags & TUI_WF_ENABLED)) {
        lb = '(';
        rb = ')';
        conio_scrnCol = (unsigned int)((0x01 << 4) | 0x01);
    } else if (w->flags & TUI_WF_FOCUSED) {
        lb = '<';
        rb = '>';
    } else {
        lb = '[';
        rb = ']';
    }
    int label_len = tui_strlen(d->label);
    int text_w = w->w - 2;
    int pad = (text_w - label_len) / 2;
    if (pad < 0) pad = 0;
    int addr = gImage + sy * displayWidth + sx;
    bk_tui_vdpchar(addr, lb);
    vdpmemset(addr + 1, ' ', w->w - 2);
    bk_tui_vdpchar(addr + w->w - 1, rb);
    if (label_len > 0 && label_len <= text_w) {
        vdpmemcpy(addr + 1 + pad, d->label, label_len);
    }
    if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
        vdpmemset(gColor + (addr - gImage), conio_scrnCol, w->w);
    }
}

int tui_button_handle_key(tui_widget_t* w, int key) {
    (void)w;
    if (key == 13 || key == 32) return 1;
    return 0;
}
