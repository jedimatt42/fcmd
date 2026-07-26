#include "banks.h"
#define MYBANK BANK(12)

#include "tui_internal.h"
#include "conio.h"

tui_widget_t* tui_textfield_create(tui_win_t* win, int x, int y, int w) {
    tui_widget_t* wgt = (tui_widget_t*)tui_alloc(sizeof(tui_widget_t) + sizeof(struct tui_textfield_data));
    if (!wgt) return 0;
    wgt->type = TUI_WIDGET_TEXTFIELD;
    wgt->x = x;
    wgt->y = y;
    wgt->w = w;
    wgt->h = 1;
    wgt->flags = TUI_WF_VISIBLE | TUI_WF_ENABLED;
    wgt->parent = win;
    wgt->next = win->children;
    win->children = wgt;
    struct tui_textfield_data* d = (struct tui_textfield_data*)tui_widget_data(wgt);
    d->buffer[0] = 0;
    d->cursor = 0;
    d->len = 0;
    bk_tui_render_widget(wgt);
    return wgt;
}

void tui_textfield_set(tui_widget_t* tf, const char* text) {
    if (!tf || tf->type != TUI_WIDGET_TEXTFIELD) return;
    struct tui_textfield_data* d = (struct tui_textfield_data*)tui_widget_data(tf);
    tui_strncpy(d->buffer, text, 81);
    d->len = tui_strlen(d->buffer);
    d->cursor = d->len;
    bk_tui_render_widget(tf);
}

const char* tui_textfield_get(tui_widget_t* tf) {
    if (!tf || tf->type != TUI_WIDGET_TEXTFIELD) return "";
    struct tui_textfield_data* d = (struct tui_textfield_data*)tui_widget_data(tf);
    return d->buffer;
}

void tui_textfield_set_cursor(tui_widget_t* tf, int pos) {
    if (!tf || tf->type != TUI_WIDGET_TEXTFIELD) return;
    struct tui_textfield_data* d = (struct tui_textfield_data*)tui_widget_data(tf);
    if (pos < 0) pos = 0;
    if (pos > d->len) pos = d->len;
    d->cursor = pos;
    bk_tui_render_widget(tf);
}

void tui_textfield_render(tui_widget_t* w) {
    struct tui_textfield_data* d = (struct tui_textfield_data*)tui_widget_data(w);
    int sx = w->parent->cx + w->x;
    int sy = w->parent->cy + w->y;
    int addr = gImage + sy * displayWidth + sx;
    int scroll_off = 0;
    if (d->cursor >= w->w) {
        scroll_off = d->cursor - w->w + 1;
    }
    int vis = d->len - scroll_off;
    if (vis > w->w) vis = w->w;
    if (vis > 0) {
        vdpmemcpy(addr, d->buffer + scroll_off, vis);
    }
    if (vis < w->w) {
        vdpmemset(addr + vis, ' ', w->w - vis);
    }
    if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
        vdpmemset(gColor + (addr - gImage), conio_scrnCol, w->w);
    }
}

int tui_textfield_handle_key(tui_widget_t* w, int key) {
    struct tui_textfield_data* d = (struct tui_textfield_data*)tui_widget_data(w);
    int redraw = 0;
    switch (key) {
        case 15:
            if (d->cursor > 0) {
                for (int i = d->cursor - 1; i < d->len; i++) {
                    d->buffer[i] = d->buffer[i + 1];
                }
                d->cursor--;
                d->len--;
                redraw = 1;
            }
            break;
        case 3:
            if (d->cursor < d->len) {
                for (int i = d->cursor; i < d->len; i++) {
                    d->buffer[i] = d->buffer[i + 1];
                }
                d->len--;
                redraw = 1;
            }
            break;
        case 7:
            d->buffer[0] = 0;
            d->cursor = 0;
            d->len = 0;
            redraw = 1;
            break;
        case 9:
            if (d->cursor < d->len) d->cursor++;
            redraw = 1;
            break;
        case 8:
            if (d->cursor > 0) d->cursor--;
            redraw = 1;
            break;
        case 147:
            d->cursor = 0;
            redraw = 1;
            break;
        case 132:
            d->cursor = d->len;
            redraw = 1;
            break;
        default:
            if (key >= 32 && key <= 126) {
                if (d->len < 80) {
                    if (d->cursor < d->len) {
                        for (int i = d->len; i > d->cursor; i--) {
                            d->buffer[i] = d->buffer[i - 1];
                        }
                    }
                    d->buffer[d->cursor] = (char)key;
                    d->cursor++;
                    d->len++;
                    d->buffer[d->len] = 0;
                    redraw = 1;
                }
            }
            break;
    }
    if (redraw) {
        d->buffer[d->len] = 0;
        bk_tui_render_widget(w);
    }
    return 0;
}
