#include "banks.h"
#define MYBANK BANK(11)

#include "tui_internal.h"
#include "conio.h"

void tui_widget_destroy(tui_widget_t* w) {
    if (!w || !w->parent) return;
    tui_widget_t** prev = &w->parent->children;
    while (*prev && *prev != w) {
        prev = &(*prev)->next;
    }
    if (*prev) {
        *prev = w->next;
    }
    if (w->parent->focus == w) {
        w->parent->focus = 0;
    }
    if (tui_ctx->focus == w) {
        tui_ctx->focus = 0;
    }
}

void tui_widget_show(tui_widget_t* w) {
    if (!w) return;
    w->flags |= TUI_WF_VISIBLE;
    tui_render_widget(w);
}

void tui_widget_hide(tui_widget_t* w) {
    if (!w) return;
    w->flags &= ~TUI_WF_VISIBLE;
    if (w->parent) {
        tui_fill(w->parent->cx + w->x, w->parent->cy + w->y, w->w, w->h, ' ');
    }
}

void tui_widget_focus(tui_widget_t* w) {
    if (!w) return;
    if (!(w->flags & TUI_WF_ENABLED)) return;
    if (tui_ctx->focus && tui_ctx->focus != w) {
        tui_widget_t* old = tui_ctx->focus;
        old->flags &= ~TUI_WF_FOCUSED;
        tui_render_widget(old);
    }
    w->flags |= TUI_WF_FOCUSED;
    tui_ctx->focus = w;
    if (w->parent) {
        w->parent->focus = w;
    }
    tui_render_widget(w);
}

void tui_widget_enable(tui_widget_t* w) {
    if (!w) return;
    w->flags |= TUI_WF_ENABLED;
    tui_render_widget(w);
}

void tui_widget_disable(tui_widget_t* w) {
    if (!w) return;
    w->flags &= ~TUI_WF_ENABLED;
    w->flags &= ~TUI_WF_FOCUSED;
    if (tui_ctx->focus == w) {
        tui_ctx->focus = 0;
    }
    tui_render_widget(w);
}

void tui_set_focus(tui_widget_t* w) {
    tui_widget_focus(w);
}

tui_widget_t* tui_get_focus() {
    return tui_ctx->focus;
}

tui_widget_t* tui_focus_next() {
    tui_win_t* win = tui_ctx->windows;
    while (win) {
        tui_widget_t* w = win->children;
        int found_current = 0;
        while (w) {
            if (w->flags & TUI_WF_VISIBLE && w->flags & TUI_WF_ENABLED) {
                if (found_current) {
                    tui_widget_focus(w);
                    return w;
                }
                if (w == tui_ctx->focus) {
                    found_current = 1;
                }
            }
            w = w->next;
        }
        if (found_current) {
            tui_widget_t* w2 = win->children;
            while (w2) {
                if (w2->flags & TUI_WF_VISIBLE && w2->flags & TUI_WF_ENABLED) {
                    tui_widget_focus(w2);
                    return w2;
                }
                w2 = w2->next;
            }
        }
        win = win->next;
    }
    return 0;
}

tui_widget_t* tui_focus_prev() {
    tui_widget_t* prev_found = 0;
    tui_widget_t* first = 0;
    tui_win_t* win = tui_ctx->windows;
    while (win) {
        tui_widget_t* w = win->children;
        int scanning = 0;
        while (w) {
            if (w->flags & TUI_WF_VISIBLE && w->flags & TUI_WF_ENABLED) {
                if (!first) first = w;
                if (w == tui_ctx->focus) {
                    if (prev_found) {
                        tui_widget_focus(prev_found);
                        return prev_found;
                    }
                    scanning = 1;
                }
                if (!scanning) {
                    prev_found = w;
                }
                if (scanning) {
                    prev_found = w;
                }
            }
            w = w->next;
        }
        win = win->next;
    }
    if (first && tui_ctx->focus != first) {
        tui_widget_focus(first);
        return first;
    }
    if (prev_found && tui_ctx->focus != prev_found) {
        tui_widget_focus(prev_found);
        return prev_found;
    }
    return 0;
}

void tui_render_widget(tui_widget_t* w) {
    if (!w || !(w->flags & TUI_WF_VISIBLE)) return;
    switch (w->type) {
        case TUI_WIDGET_BUTTON:
            bk_tui_button_render(w);
            break;
        case TUI_WIDGET_LABEL:
            bk_tui_label_render(w);
            break;
        case TUI_WIDGET_LIST:
            bk_tui_list_render(w);
            break;
        case TUI_WIDGET_TEXTFIELD:
            bk_tui_textfield_render(w);
            break;
        case TUI_WIDGET_CHECKBOX:
            bk_tui_checkbox_render(w);
            break;
        case TUI_WIDGET_PROGRESSBAR:
            bk_tui_progressbar_render(w);
            break;
    }
}

int tui_handle_widget_key(tui_widget_t* w, int key) {
    if (!w || !(w->flags & TUI_WF_VISIBLE) || !(w->flags & TUI_WF_ENABLED)) return 0;
    switch (w->type) {
        case TUI_WIDGET_BUTTON:
            return bk_tui_button_handle_key(w, key);
        case TUI_WIDGET_LABEL:
            return 0;
        case TUI_WIDGET_LIST:
            return bk_tui_list_handle_key(w, key);
        case TUI_WIDGET_TEXTFIELD:
            return bk_tui_textfield_handle_key(w, key);
        case TUI_WIDGET_CHECKBOX:
            return bk_tui_checkbox_handle_key(w, key);
        case TUI_WIDGET_PROGRESSBAR:
            return 0;
    }
    return 0;
}
