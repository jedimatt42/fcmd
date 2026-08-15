#include "banks.h"
#define MYBANK BANK(11)

#include "tui_internal.h"
#include "globals.h"
#include "vdp.h"
#include "conio.h"

static void tui_win_draw_box(tui_win_t* win) {
    conio_scrnCol = (unsigned int)((win->fg << 4) | (win->bg & 0x0F));
    int flags = win->border_flags;
    int lastcol = win->x + win->w - 1;
    int lastrow = win->y + win->h - 1;
    int rs = displayWidth;
    if (flags & TUI_BF_TOP) {
        tui_vdpchar(gImage + win->y * rs + win->x, 0xDA);
        vdpmemset(gImage + win->y * rs + win->x + 1, 0xC4, win->w - 2);
        tui_vdpchar(gImage + win->y * rs + lastcol, 0xBF);
        if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
            vdpmemset(gColor + win->y * rs + win->x, conio_scrnCol, win->w);
        }
        if (win->title[0]) {
            int avail = win->w - 2;
            int title_len = tui_strlen(win->title);
            int start_x = win->x + 1 + (avail - title_len) / 2;
            if (start_x < win->x + 1) start_x = win->x + 1;
            vdpmemcpy(gImage + win->y * rs + start_x, win->title, title_len);
        }
    }
    if (flags & TUI_BF_SIDES) {
        int bs = win->y + ((flags & TUI_BF_TOP) ? 1 : 0);
        int be = win->y + win->h - ((flags & TUI_BF_BOTTOM) ? 1 : 0);
        for (int r = bs; r < be; r++) {
            tui_vdpchar(gImage + r * rs + win->x, 0xB3);
            tui_vdpchar(gImage + r * rs + lastcol, 0xB3);
        }
    }
    if (flags & TUI_BF_BOTTOM) {
        tui_vdpchar(gImage + lastrow * rs + win->x, 0xC0);
        vdpmemset(gImage + lastrow * rs + win->x + 1, 0xC4, win->w - 2);
        tui_vdpchar(gImage + lastrow * rs + lastcol, 0xD9);
        if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
            vdpmemset(gColor + lastrow * rs + win->x, conio_scrnCol, win->w);
        }
    }
}

tui_win_t* tui_win_open(int x, int y, int w, int h) {
    tui_win_t* win = (tui_win_t*)tui_alloc(sizeof(tui_win_t));
    if (!win) return 0;
    win->x = x;
    win->y = y;
    win->w = w;
    win->h = h;
    win->border_flags = TUI_BF_ALL;
    win->cx = x + 1;
    win->cy = y + 1;
    win->cw = w - 2;
    win->ch = h - 2;
    win->fg = 0x0F;
    win->bg = 0x01;
    win->title[0] = 0;
    win->children = 0;
    win->focus = 0;
    win->next = tui_ctx->windows;
    tui_ctx->windows = win;
    conio_scrnCol = (unsigned int)((win->fg << 4) | (win->bg & 0x0F));
    tui_fill(win->x, win->y, win->w, win->h, ' ');
    tui_win_draw_box(win);
    return win;
}

void tui_win_close(tui_win_t* win) {
    if (!win) return;
    tui_win_t** prev = &tui_ctx->windows;
    while (*prev && *prev != win) {
        prev = &(*prev)->next;
    }
    if (*prev) {
        *prev = win->next;
    }
    if (tui_ctx->focus && tui_ctx->focus->parent == win) {
        tui_ctx->focus = 0;
    }
    tui_fill(win->x, win->y, win->w, win->h, ' ');

    tui_win_t* stack[10];
    int count = 0;
    tui_win_t* w = tui_ctx->windows;
    while (w && count < 10) {
        stack[count++] = w;
        w = w->next;
    }
    for (int i = count - 1; i >= 0; i--) {
        tui_win_draw_box(stack[i]);
        tui_widget_t* child = stack[i]->children;
        while (child) {
            tui_render_widget(child);
            child = child->next;
        }
    }

    if (!tui_ctx->focus && tui_ctx->windows) {
        tui_win_t* top = tui_ctx->windows;
        if (top->focus && (top->focus->flags & TUI_WF_ENABLED)) {
            tui_widget_focus(top->focus);
        }
    }
}

void tui_win_set_title(tui_win_t* win, const char* title) {
    if (!win) return;
    tui_strncpy(win->title, title, 41);
    tui_win_draw_box(win);
}

void tui_win_move(tui_win_t* win, int x, int y) {
    if (!win) return;
    tui_fill(win->x, win->y, win->w, win->h, ' ');
    win->x = x;
    win->y = y;
    win->cx = x + ((win->border_flags & TUI_BF_SIDES) ? 1 : 0);
    win->cy = y + ((win->border_flags & TUI_BF_TOP) ? 1 : 0);
    tui_win_draw_box(win);
    tui_widget_t* w = win->children;
    while (w) {
        tui_render_widget(w);
        w = w->next;
    }
}

void tui_win_resize(tui_win_t* win, int w, int h) {
    if (!win) return;
    tui_fill(win->x, win->y, win->w, win->h, ' ');
    win->w = w;
    win->h = h;
    win->cw = w - ((win->border_flags & TUI_BF_SIDES) ? 2 : 0);
    win->ch = h - ((win->border_flags & TUI_BF_TOP) ? 1 : 0) - ((win->border_flags & TUI_BF_BOTTOM) ? 1 : 0);
    tui_win_draw_box(win);
    tui_widget_t* child = win->children;
    while (child) {
        tui_render_widget(child);
        child = child->next;
    }
}

void tui_win_set_colors(tui_win_t* win, int fg, int bg) {
    if (!win) return;
    win->fg = fg;
    win->bg = bg;
}

void tui_win_set_border(tui_win_t* win, int flags) {
    if (!win) return;
    win->border_flags = (unsigned char)flags;
    win->cx = win->x + ((flags & TUI_BF_SIDES) ? 1 : 0);
    win->cy = win->y + ((flags & TUI_BF_TOP) ? 1 : 0);
    win->cw = win->w - ((flags & TUI_BF_SIDES) ? 2 : 0);
    win->ch = win->h - ((flags & TUI_BF_TOP) ? 1 : 0) - ((flags & TUI_BF_BOTTOM) ? 1 : 0);
}

void tui_win_gotoxy(tui_win_t* win, int x, int y) {
    if (!win) return;
    gotoxy(win->cx + x, win->cy + y);
}

void tui_win_putc(tui_win_t* win, int c) {
    (void)win;
    cputc(c);
}

void tui_win_puts(tui_win_t* win, const char* s) {
    (void)win;
    cputs(s);
}

static const char* tui_parse_int(const char* fmt, int* val) {
    *val = 0;
    while (*fmt >= '0' && *fmt <= '9') {
        *val = (*val * 10) + (*fmt - '0');
        fmt++;
    }
    return fmt;
}

static void tui_reverse(char* s, int len) {
    int i = 0;
    int j = len - 1;
    while (i < j) {
        char t = s[i];
        s[i] = s[j];
        s[j] = t;
        i++; j--;
    }
}

static int tui_utoa(unsigned int n, char* buf) {
    int i = 0;
    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n > 0) {
            buf[i++] = '0' + (n % 10);
            n /= 10;
        }
    }
    tui_reverse(buf, i);
    buf[i] = 0;
    return i;
}

void tui_win_printf(tui_win_t* win, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[80];
    while (*fmt) {
        if (*fmt != '%') {
            buf[0] = *fmt;
            buf[1] = 0;
            tui_win_puts(win, buf);
            fmt++;
            continue;
        }
        fmt++;
        switch (*fmt) {
            case 's': {
                const char* s = va_arg(args, const char*);
                tui_win_puts(win, s ? s : "(null)");
                break;
            }
            case 'd': {
                int n = va_arg(args, int);
                if (n < 0) {
                    tui_win_putc(win, '-');
                    n = -n;
                }
                tui_utoa((unsigned int)n, buf);
                tui_win_puts(win, buf);
                break;
            }
            case 'u': {
                unsigned int n = va_arg(args, unsigned int);
                tui_utoa(n, buf);
                tui_win_puts(win, buf);
                break;
            }
            case '%':
                tui_win_putc(win, '%');
                break;
            default:
                tui_win_putc(win, '%');
                tui_win_putc(win, *fmt);
                break;
        }
        if (*fmt) fmt++;
    }
    va_end(args);
}

void tui_win_scroll(tui_win_t* win, int lines) {
    if (!win || lines == 0 || win->ch <= 0) return;
    int rowstride = displayWidth;
    int src_line, dst_line, count;
    if (lines > 0) {
        src_line = win->cy + lines;
        dst_line = win->cy;
        count = win->ch - lines;
    } else {
        src_line = win->cy;
        dst_line = win->cy - lines;
        count = win->ch + lines;
        lines = -lines;
    }
    int src_addr = gImage + src_line * rowstride + win->cx;
    int dst_addr = gImage + dst_line * rowstride + win->cx;
    int bytes = count * rowstride;
    vdpmemcpy(dst_addr, (const char*)src_addr, bytes);
    if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
        bytes = count * rowstride;
        vdpmemcpy(gColor + (dst_addr - gImage), (const char*)(gColor + (src_addr - gImage)), bytes);
    }
    if (lines > 0) {
        int clear_addr = gImage + (win->cy + win->ch - lines) * rowstride + win->cx;
        vdpmemset(clear_addr, ' ', lines * rowstride);
        if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
            vdpmemset(gColor + (clear_addr - gImage), conio_scrnCol, lines * rowstride);
        }
    } else {
        int clear_addr = gImage + win->cy * rowstride + win->cx;
        vdpmemset(clear_addr, ' ', (-lines) * rowstride);
        if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
            vdpmemset(gColor + (clear_addr - gImage), conio_scrnCol, (-lines) * rowstride);
        }
    }
}
