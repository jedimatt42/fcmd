#include "banks.h"
#define MYBANK BANK(11)

#include "tui_internal.h"
#include "globals.h"
#include "vdp.h"
#include "conio.h"

void tui_gotoxy(int x, int y) {
    gotoxy(x, y);
}

void tui_vdpchar(int pAddr, int ch) {
    VDP_SET_ADDRESS_WRITE(pAddr);
    VDPWD = ch;
    if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
        VDP_SET_ADDRESS_WRITE(gColor + (pAddr - gImage));
        VDPWD = (unsigned char)conio_scrnCol;
    }
}

static void outchar(int c) {
    int addr = gImage + conio_y * displayWidth + conio_x;
    tui_vdpchar(addr, c);
    conio_x++;
    if (conio_x >= displayWidth) {
        conio_x = 0;
        conio_y++;
        if (conio_y >= displayHeight) conio_y = displayHeight - 1;
    }
}

void tui_putc(int c) {
    outchar(c);
}

void tui_puts(const char* s) {
    while (*s) outchar(*s++);
}

void tui_set_color(int fg, int bg) {
    conio_scrnCol = (unsigned int)((fg << 4) | (bg & 0x0F));
}

void tui_hline(int x, int y, int w) {
    int addr = gImage + y * displayWidth + x;
    vdpmemset(addr, 0xC4, w);
    if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
        vdpmemset(gColor + (addr - gImage), conio_scrnCol, w);
    }
}

void tui_vline(int x, int y, int h) {
    for (int i = 0; i < h; i++) {
        int addr = gImage + (y + i) * displayWidth + x;
        tui_vdpchar(addr, 0xB3);
    }
}

void tui_box(int x, int y, int w, int h) {
    if (w < 2 || h < 2) return;
    int lastcol = x + w - 1;
    int lastrow = y + h - 1;
    int rowstride = displayWidth;

    tui_vdpchar(gImage + y * rowstride + x, 0xDA);
    vdpmemset(gImage + y * rowstride + x + 1, 0xC4, w - 2);
    tui_vdpchar(gImage + y * rowstride + lastcol, 0xBF);

    for (int r = y + 1; r < lastrow; r++) {
        tui_vdpchar(gImage + r * rowstride + x, 0xB3);
        vdpmemset(gImage + r * rowstride + x + 1, ' ', w - 2);
        tui_vdpchar(gImage + r * rowstride + lastcol, 0xB3);
    }

    tui_vdpchar(gImage + lastrow * rowstride + x, 0xC0);
    vdpmemset(gImage + lastrow * rowstride + x + 1, 0xC4, w - 2);
    tui_vdpchar(gImage + lastrow * rowstride + lastcol, 0xD9);
}

void tui_box_title(int x, int y, int w, int h, const char* title) {
    tui_box(x, y, w, h);
    if (!title || !*title) return;
    int title_len = tui_strlen(title);
    int avail = w - 2;
    int start_x = x + 1 + (avail - title_len) / 2;
    if (start_x < x + 1) start_x = x + 1;
    int addr = gImage + y * displayWidth + start_x;
    vdpmemcpy(addr, title, title_len);
}

void tui_fill(int x, int y, int w, int h, int ch) {
    int rowstride = displayWidth;
    for (int r = 0; r < h; r++) {
        int addr = gImage + (y + r) * rowstride + x;
        vdpmemset(addr, ch, w);
        if (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) {
            vdpmemset(gColor + (addr - gImage), conio_scrnCol, w);
        }
    }
}
