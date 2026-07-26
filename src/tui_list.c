#include "banks.h"
#define MYBANK BANK(12)

#include "tui_internal.h"
#include "conio.h"

tui_widget_t* tui_list_create(tui_win_t* win, int x, int y, int w, int h) {
    tui_widget_t* wgt = (tui_widget_t*)tui_alloc(sizeof(tui_widget_t) + sizeof(struct tui_list_data));
    if (!wgt) return 0;
    wgt->type = TUI_WIDGET_LIST;
    wgt->x = x;
    wgt->y = y;
    wgt->w = w;
    wgt->h = h;
    wgt->flags = TUI_WF_VISIBLE | TUI_WF_ENABLED;
    wgt->parent = win;
    wgt->next = win->children;
    win->children = wgt;
    struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(wgt);
    d->count = 0;
    d->selected = -1;
    d->scroll_offset = 0;
    d->head = 0;
    d->tail = 0;
    bk_tui_render_widget(wgt);
    return wgt;
}

static struct tui_list_item* tui_list_item_at(tui_widget_t* list, int idx) {
    struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(list);
    struct tui_list_item* item = d->head;
    for (int i = 0; i < idx && item; i++) {
        item = item->next;
    }
    return item;
}

int tui_list_add(tui_widget_t* list, const char* item) {
    if (!list || list->type != TUI_WIDGET_LIST) return -1;
    struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(list);
    struct tui_list_item* li = (struct tui_list_item*)tui_alloc(sizeof(struct tui_list_item));
    if (!li) return -1;
    tui_strncpy(li->text, item, 40);
    li->next = 0;
    if (d->tail) {
        d->tail->next = li;
    } else {
        d->head = li;
    }
    d->tail = li;
    int idx = d->count;
    d->count++;
    bk_tui_render_widget(list);
    return idx;
}

int tui_list_insert(tui_widget_t* list, int idx, const char* item) {
    if (!list || list->type != TUI_WIDGET_LIST || idx < 0) return -1;
    struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(list);
    if (idx > d->count) return -1;
    struct tui_list_item* li = (struct tui_list_item*)tui_alloc(sizeof(struct tui_list_item));
    if (!li) return -1;
    tui_strncpy(li->text, item, 40);
    if (idx == 0) {
        li->next = d->head;
        d->head = li;
        if (!d->tail) d->tail = li;
    } else {
        struct tui_list_item* prev = tui_list_item_at(list, idx - 1);
        if (!prev) return -1;
        li->next = prev->next;
        prev->next = li;
        if (!li->next) d->tail = li;
    }
    if (d->selected >= idx) d->selected++;
    d->count++;
    bk_tui_render_widget(list);
    return idx;
}

void tui_list_remove(tui_widget_t* list, int idx) {
    if (!list || list->type != TUI_WIDGET_LIST || idx < 0) return;
    struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(list);
    if (idx >= d->count) return;
    struct tui_list_item* prev = 0;
    struct tui_list_item* cur = d->head;
    for (int i = 0; i < idx && cur; i++) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) return;
    if (prev) {
        prev->next = cur->next;
    } else {
        d->head = cur->next;
    }
    if (!cur->next) d->tail = prev;
    d->count--;
    if (d->selected == idx) d->selected = -1;
    else if (d->selected > idx) d->selected--;
    if (d->scroll_offset > 0 && d->scroll_offset >= d->count) {
        d->scroll_offset = d->count > list->h ? d->count - list->h : 0;
    }
    bk_tui_render_widget(list);
}

void tui_list_clear(tui_widget_t* list) {
    if (!list || list->type != TUI_WIDGET_LIST) return;
    struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(list);
    d->count = 0;
    d->selected = -1;
    d->scroll_offset = 0;
    d->head = 0;
    d->tail = 0;
    bk_tui_render_widget(list);
}

int tui_list_count(tui_widget_t* list) {
    if (!list || list->type != TUI_WIDGET_LIST) return 0;
    struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(list);
    return d->count;
}

int tui_list_get_selected(tui_widget_t* list) {
    if (!list || list->type != TUI_WIDGET_LIST) return -1;
    struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(list);
    return d->selected;
}

void tui_list_set_selected(tui_widget_t* list, int idx) {
    if (!list || list->type != TUI_WIDGET_LIST) return;
    struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(list);
    if (idx < -1 || idx >= d->count) return;
    d->selected = idx;
    if (idx < d->scroll_offset) {
        d->scroll_offset = idx;
    } else if (idx >= d->scroll_offset + list->h) {
        d->scroll_offset = idx - list->h + 1;
    }
    bk_tui_render_widget(list);
}

void tui_list_render(tui_widget_t* w) {
    struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(w);
    int sx = w->parent->cx + w->x;
    int sy = w->parent->cy + w->y;
    int rowstride = displayWidth;
    int scrollbar_w = (d->count > w->h) ? 1 : 0;
    int item_w = w->w - scrollbar_w;

    for (int r = 0; r < w->h; r++) {
        int idx = d->scroll_offset + r;
        int addr = gImage + (sy + r) * rowstride + sx;
        if (idx < d->count) {
            struct tui_list_item* li = tui_list_item_at(w, idx);
            if (w->flags & TUI_WF_FOCUSED && idx == d->selected) {
                bk_tui_vdpchar(addr, 0x10);
            } else {
                bk_tui_vdpchar(addr, ' ');
            }
            int text_len = tui_strlen(li->text);
            int copy_len = text_len < item_w - 1 ? text_len : item_w - 1;
            if (copy_len > 0) {
                vdpmemcpy(addr + 1, li->text, copy_len);
            }
            if (copy_len < item_w - 1) {
                vdpmemset(addr + 1 + copy_len, ' ', item_w - 1 - copy_len);
            }
        } else {
            vdpmemset(addr, ' ', item_w);
        }
        if (scrollbar_w) {
            int thumb_pos = (d->count > w->h)
                ? (d->scroll_offset * (w->h - 1)) / (d->count - w->h)
                : 0;
            bk_tui_vdpchar(addr + item_w, (r == thumb_pos) ? 0xDB : 0xB0);
        }
        if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
            vdpmemset(gColor + (addr - gImage), conio_scrnCol, w->w);
        }
    }
}

int tui_list_handle_key(tui_widget_t* w, int key) {
    struct tui_list_data* d = (struct tui_list_data*)tui_widget_data(w);
    int old = d->selected;
    switch (key) {
        case 10:
            if (d->selected < d->count - 1) {
                d->selected++;
                if (d->selected >= d->scroll_offset + w->h) {
                    d->scroll_offset++;
                }
            }
            break;
        case 11:
            if (d->selected > 0) {
                d->selected--;
                if (d->selected < d->scroll_offset) {
                    d->scroll_offset--;
                }
            }
            break;
        case 147:
            d->selected = 0;
            d->scroll_offset = 0;
            break;
        case 132:
            d->selected = d->count - 1;
            d->scroll_offset = d->count > w->h ? d->count - w->h : 0;
            break;
        case 13:
        case 32:
            if (d->selected >= 0) return 1;
            return 0;
        default:
            return 0;
    }
    if (old != d->selected) {
        bk_tui_render_widget(w);
    }
    return 0;
}
