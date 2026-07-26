#include "banks.h"
#define MYBANK BANK(12)

#include "tui_internal.h"
#include "conio.h"

tui_widget_t* tui_label_create(tui_win_t* win, int x, int y, const char* text) {
    tui_widget_t* wgt = (tui_widget_t*)tui_alloc(sizeof(tui_widget_t) + sizeof(struct tui_label_data));
    if (!wgt) return 0;
    wgt->type = TUI_WIDGET_LABEL;
    wgt->x = x;
    wgt->y = y;
    wgt->w = 0;
    wgt->h = 1;
    wgt->flags = TUI_WF_VISIBLE;
    wgt->parent = win;
    wgt->next = win->children;
    win->children = wgt;
    struct tui_label_data* d = (struct tui_label_data*)tui_widget_data(wgt);
    if (text) {
        tui_strncpy(d->text, text, 81);
        wgt->w = tui_strlen(d->text);
    } else {
        d->text[0] = 0;
        wgt->w = 0;
    }
    bk_tui_render_widget(wgt);
    return wgt;
}

void tui_label_set_text(tui_widget_t* lbl, const char* text) {
    if (!lbl || lbl->type != TUI_WIDGET_LABEL) return;
    struct tui_label_data* d = (struct tui_label_data*)tui_widget_data(lbl);
    if (text) {
        tui_strncpy(d->text, text, 81);
        lbl->w = tui_strlen(d->text);
    } else {
        d->text[0] = 0;
        lbl->w = 0;
    }
    bk_tui_render_widget(lbl);
}

void tui_label_render(tui_widget_t* w) {
    struct tui_label_data* d = (struct tui_label_data*)tui_widget_data(w);
    int sx = w->parent->cx + w->x;
    int sy = w->parent->cy + w->y;
    int addr = gImage + sy * displayWidth + sx;
    int len = tui_strlen(d->text);
    if (len > 0) {
        vdpmemcpy(addr, d->text, len);
    }
    if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
        vdpmemset(gColor + (addr - gImage), conio_scrnCol, len);
    }
}
