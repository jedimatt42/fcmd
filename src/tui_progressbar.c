#include "banks.h"
#define MYBANK BANK(12)

#include "tui_internal.h"
#include "conio.h"

tui_widget_t* tui_progressbar_create(tui_win_t* win, int x, int y, int w) {
    tui_widget_t* wgt = (tui_widget_t*)tui_alloc(sizeof(tui_widget_t) + sizeof(struct tui_progressbar_data));
    if (!wgt) return 0;
    wgt->type = TUI_WIDGET_PROGRESSBAR;
    wgt->x = x;
    wgt->y = y;
    wgt->w = w;
    wgt->h = 1;
    wgt->flags = TUI_WF_VISIBLE;
    wgt->parent = win;
    wgt->next = win->children;
    win->children = wgt;
    struct tui_progressbar_data* d = (struct tui_progressbar_data*)tui_widget_data(wgt);
    d->value = 0;
    tui_render_widget(wgt);
    return wgt;
}

void tui_progressbar_set(tui_widget_t* pb, int value) {
    if (!pb || pb->type != TUI_WIDGET_PROGRESSBAR) return;
    struct tui_progressbar_data* d = (struct tui_progressbar_data*)tui_widget_data(pb);
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    d->value = value;
    tui_render_widget(pb);
}

void tui_progressbar_render(tui_widget_t* w) {
    struct tui_progressbar_data* d = (struct tui_progressbar_data*)tui_widget_data(w);
    int sx = w->parent->cx + w->x;
    int sy = w->parent->cy + w->y;
    int addr = gImage + sy * displayWidth + sx;
    int fill_w = w->w - 2;
    if (fill_w < 1) fill_w = 1;
    int filled = (d->value * fill_w) / 100;
    vdpchar(addr, '[');
    if (filled > 0) {
        vdpmemset(addr + 1, 0xDB, filled);
    }
    int empty = fill_w - filled;
    if (empty > 0) {
        vdpmemset(addr + 1 + filled, 0x2D, empty);
    }
    vdpchar(addr + w->w - 1, ']');
    if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
        vdpmemset(gColor + (addr - gImage), conio_scrnCol, w->w);
    }
}
