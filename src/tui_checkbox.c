#include "banks.h"
#define MYBANK BANK(12)

#include "tui_internal.h"
#include "conio.h"

tui_widget_t* tui_checkbox_create(tui_win_t* win, int x, int y, const char* label) {
    tui_widget_t* wgt = (tui_widget_t*)tui_alloc(sizeof(tui_widget_t) + sizeof(struct tui_checkbox_data));
    if (!wgt) return 0;
    wgt->type = TUI_WIDGET_CHECKBOX;
    wgt->x = x;
    wgt->y = y;
    wgt->w = 4 + (label ? tui_strlen(label) : 0);
    wgt->h = 1;
    wgt->flags = TUI_WF_VISIBLE | TUI_WF_ENABLED;
    wgt->parent = win;
    wgt->next = win->children;
    win->children = wgt;
    struct tui_checkbox_data* d = (struct tui_checkbox_data*)tui_widget_data(wgt);
    if (label) {
        tui_strncpy(d->label, label, 41);
    } else {
        d->label[0] = 0;
    }
    d->state = 0;
    tui_render_widget(wgt);
    return wgt;
}

void tui_checkbox_set(tui_widget_t* cb, int state) {
    if (!cb || cb->type != TUI_WIDGET_CHECKBOX) return;
    struct tui_checkbox_data* d = (struct tui_checkbox_data*)tui_widget_data(cb);
    d->state = state ? 1 : 0;
    tui_render_widget(cb);
}

int tui_checkbox_get(tui_widget_t* cb) {
    if (!cb || cb->type != TUI_WIDGET_CHECKBOX) return 0;
    struct tui_checkbox_data* d = (struct tui_checkbox_data*)tui_widget_data(cb);
    return d->state;
}

void tui_checkbox_render(tui_widget_t* w) {
    struct tui_checkbox_data* d = (struct tui_checkbox_data*)tui_widget_data(w);
    int sx = w->parent->cx + w->x;
    int sy = w->parent->cy + w->y;
    int addr = gImage + sy * displayWidth + sx;
    if (!(w->flags & TUI_WF_ENABLED)) {
        vdpchar(addr, '(');
        vdpchar(addr + 1, d->state ? '*' : ' ');
        vdpchar(addr + 2, ')');
    } else if (w->flags & TUI_WF_FOCUSED) {
        vdpchar(addr, '<');
        vdpchar(addr + 1, d->state ? '*' : ' ');
        vdpchar(addr + 2, '>');
    } else {
        vdpchar(addr, '[');
        vdpchar(addr + 1, d->state ? '*' : ' ');
        vdpchar(addr + 2, ']');
    }
    vdpchar(addr + 3, ' ');
    int label_len = tui_strlen(d->label);
    if (label_len > 0) {
        vdpmemcpy(addr + 4, d->label, label_len);
    }
    if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
        vdpmemset(gColor + (addr - gImage), conio_scrnCol, 4 + label_len);
    }
}

int tui_checkbox_handle_key(tui_widget_t* w, int key) {
    if (key == 13 || key == 32) {
        struct tui_checkbox_data* d = (struct tui_checkbox_data*)tui_widget_data(w);
        d->state = d->state ? 0 : 1;
        tui_render_widget(w);
        return 1;
    }
    return 0;
}
