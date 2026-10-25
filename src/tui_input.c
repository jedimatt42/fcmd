#include "banks.h"
#define MYBANK BANK(11)

#include "tui_internal.h"
#include "terminal.h"
#include "tipi_mouse.h"
#include "editor.h"
#include "kscan.h"
#include "detect_vdp.h"

/* The TIPI mouse sprite uses the F18A 256x192 coordinate space. A character
   cell is (240 / displayWidth) x (240 / displayHeight) pixels; the text area
   starts 8 pixels in from the left. */
#define TUI_MOUSE_X0 8

static int tui_mouse_available(void) {
    /* Mirrors SUPPORT_MOUSE in tipi_mouse.c: the pointer is F18A 80x30 only. */
    return (vdp_type == VDP_F18A && displayHeight == 30) ? 1 : 0;
}

static void tui_mouse_cell(int* col, int* row);

int tui_mouse_enable(void) {
    if (!tui_ctx) return 0;
    if (tui_ctx->mouse_depth) {
        tui_ctx->mouse_depth++;
        return 1;
    }
    if (!tui_mouse_available()) return 0;
    int c, r;
    tui_ctx->mouse.mx = 0;
    tui_ctx->mouse.my = 0;
    tui_ctx->mouse.buttons = 0;
    bk_enablePointer(&tui_ctx->mouse);
    tui_ctx->mouse_on = 1;
    tui_ctx->mouse_depth = 1;
    tui_ctx->mouse_prev = 0;
    tui_mouse_cell(&c, &r);
    tui_ctx->mouse_col = (unsigned char)c;
    tui_ctx->mouse_row = (unsigned char)r;
    return 1;
}

void tui_mouse_disable(void) {
    if (!tui_ctx || !tui_ctx->mouse_depth) return;
    tui_ctx->mouse_depth--;
    if (tui_ctx->mouse_depth) return;
    bk_disablePointer();
    tui_ctx->mouse_on = 0;
    tui_ctx->mouse_prev = 0;
}

/* Translate the sprite position into a character cell. */
static void tui_mouse_cell(int* col, int* row) {
    int cw = 240 / (int)displayWidth;
    int rh = 240 / (int)displayHeight;
    int c = (tui_ctx->mouse.pointerx - TUI_MOUSE_X0) / cw;
    int r = tui_ctx->mouse.pointery / rh;
    if (c < 0) c = 0;
    if (c >= (int)displayWidth) c = (int)displayWidth - 1;
    if (r < 0) r = 0;
    if (r >= (int)displayHeight) r = (int)displayHeight - 1;
    *col = c;
    *row = r;
}

/* Topmost window under the cell, and the topmost widget in it (siblings later
   in the list are drawn last, so they win an overlap). Returns 0 when the cell
   is on a window border or outside every window. */
static tui_widget_t* tui_mouse_hit(int col, int row, tui_win_t** win_out) {
    tui_win_t* win = tui_ctx->windows;
    while (win) {
        if (col >= win->x && col < win->x + win->w &&
            row >= win->y && row < win->y + win->h) {
            tui_widget_t* hit = 0;
            tui_widget_t* w = win->children;
            while (w) {
                int wx = win->cx + w->x;
                int wy = win->cy + w->y;
                if ((w->flags & TUI_WF_VISIBLE) &&
                    col >= wx && col < wx + w->w &&
                    row >= wy && row < wy + w->h) {
                    hit = w;
                }
                w = w->next;
            }
            *win_out = win;
            return hit;
        }
        win = win->next;
    }
    *win_out = 0;
    return 0;
}

/* Focus the clicked widget and let it react as if ENTER was pressed. Returns 1
   when that counts as a widget activation. */
static int tui_mouse_activate(tui_widget_t* w, tui_win_t* win, int col, int row) {
    if (!w || !win) return 0;
    if (!(w->flags & TUI_WF_VISIBLE) || !(w->flags & TUI_WF_ENABLED)) return 0;

    switch (w->type) {
        case TUI_WIDGET_LIST: {
            struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(w);
            int idx = d->scroll_offset + row - (win->cy + w->y);
            if (idx < 0 || idx >= d->count) return 0;
            tui_widget_focus(w);
            bk_tui_list_set_selected(w, idx);
            return tui_handle_widget_key(w, 13);
        }
        case TUI_WIDGET_TEXTFIELD: {
            struct tui_textfield_data* d = (struct tui_textfield_data*)tui_widget_data(w);
            int scroll_off = (d->cursor >= w->w) ? d->cursor - w->w + 1 : 0;
            int pos = col - (win->cx + w->x) + scroll_off;
            if (pos < 0) pos = 0;
            if (pos > d->len) pos = d->len;
            tui_widget_focus(w);
            bk_tui_textfield_set_cursor(w, pos);
            return 0;
        }
        case TUI_WIDGET_BUTTON:
        case TUI_WIDGET_CHECKBOX:
            tui_widget_focus(w);
            return tui_handle_widget_key(w, 13);
        default:
            return 0;
    }
}

int tui_get_event(struct tui_event* ev) {
    if (!ev) return 0;
    ev->type = TUI_EV_NONE;
    ev->widget = 0;
    ev->key = 0;
    ev->mx = 0;
    ev->my = 0;
    ev->buttons = 0;

    int k = (int)bk_kscan(5);
    if (k != 255 && (KSCAN_STATUS & KSCAN_MASK)) {
        ev->key = k;
        ev->type = TUI_EV_KEY;

        if (k == 6) {
            tui_focus_next();
            return 1;
        }
        if (k == 2) {
            if (tui_ctx->windows) {
                tui_win_t* top = tui_ctx->windows;
                tui_win_t* last = top;
                while (last->next) last = last->next;
                if (last->children == 0 || tui_ctx->focus == 0) {
                    tui_win_close(last);
                    return 1;
                }
            }
        }
        if (tui_ctx->focus) {
            int handled = tui_handle_widget_key(tui_ctx->focus, k);
            if (handled) {
                ev->type = TUI_EV_WIDGET_ACTIVATED;
                ev->widget = tui_ctx->focus;
                return 1;
            }
        }
        return 1;
    }

    if (tui_ctx->mouse_on) {
        tui_ctx->mouse.mx = 0;
        tui_ctx->mouse.my = 0;
        bk_updatePointer(&tui_ctx->mouse);

        int col, row;
        tui_mouse_cell(&col, &row);
        unsigned char buttons = (unsigned char)tui_ctx->mouse.buttons;
        unsigned char pressed = buttons & ~tui_ctx->mouse_prev;
        tui_ctx->mouse_prev = buttons;

        ev->mx = col;
        ev->my = row;
        ev->buttons = buttons;

        if (pressed & MB_LEFT) {
            tui_win_t* win = 0;
            tui_widget_t* hit = tui_mouse_hit(col, row, &win);
            ev->widget = hit;
            if (hit && tui_mouse_activate(hit, win, col, row)) {
                ev->type = TUI_EV_WIDGET_ACTIVATED;
            } else {
                ev->type = TUI_EV_MOUSE_CLICK;
            }
            return 1;
        }

        if (pressed) {
            ev->type = TUI_EV_MOUSE_CLICK;
            return 1;
        }

        if (col != (int)tui_ctx->mouse_col || row != (int)tui_ctx->mouse_row) {
            tui_ctx->mouse_col = (unsigned char)col;
            tui_ctx->mouse_row = (unsigned char)row;
            ev->type = TUI_EV_MOUSE_MOVE;
            return 1;
        }
    }

    return 0;
}

int tui_dispatch_event(struct tui_event* ev) {
    if (!ev || ev->type == TUI_EV_NONE) return 0;
    return 1;
}
