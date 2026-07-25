#ifndef _terminal_h
#define _terminal_h 1

#include "banking.h"
#include "strutil.h"

unsigned int term_kscan(unsigned int mode); // from term_kscan
void gplvdp(int vect, int adr, int cnt); // from vdp_gplvdp
void vdp_setchar(int pAddr, int ch); // write a char to screen address
unsigned int vdp_cursor_addr(); // returns VDP address of cursor
void raw_cputc(int k); // set character without advancing or term code 

typedef void (*identify_callback)(int flag);

void initTerminal();
void term_set_identify_hook(identify_callback cb);
identify_callback get_identify_hook();
void term_putc(int c);
void term_puts(const char* str);
void term_gotoxy(int x, int y);
void term_cls();
unsigned int term_set_text_color(unsigned int color);
unsigned int term_set_bg_color(unsigned int color);
unsigned int term_set_border_color(unsigned int x);

DECLARE_BANKED_VOID(initTerminal, BANK(8), bk_initTerminal, (), ())
DECLARE_BANKED_VOID(term_set_identify_hook, BANK(8), bk_set_identify_hook, (identify_callback cb), (cb))
DECLARE_BANKED(get_identify_hook, BANK(8), identify_callback, bk_get_identify_hook, (), ())
DECLARE_BANKED_VOID(term_putc, BANK(8), bk_tputc, (int c), (c))
DECLARE_BANKED_VOID(term_puts, BANK(8), bk_tputs_ram, (const char* str), (str))
DECLARE_BANKED_VOID(term_cls, BANK(8), bk_term_cls, (), ())
DECLARE_BANKED_VOID(gplvdp, BANK(8), bk_gplvdp, (int vect, int adr, int cnt), (vect, adr, cnt))
DECLARE_BANKED_VOID(term_gotoxy, BANK(8), bk_cursorGoto, (int x, int y), (x, y))

DECLARE_BANKED(term_set_text_color, BANK(8), unsigned int, bk_term_set_text_color, (unsigned int color), (color))
DECLARE_BANKED(term_set_bg_color, BANK(8), unsigned int, bk_term_set_bg_color, (unsigned int color), (color))
DECLARE_BANKED(term_set_border_color, BANK(8), unsigned int, bk_term_set_border_color, (unsigned int x), (x))
DECLARE_BANKED(term_kscan, BANK(8), unsigned int, bk_kscan, (unsigned int mode), (mode))
DECLARE_BANKED_VOID(vdp_setchar, BANK(8), bk_vdp_setchar, (int pAddr, int ch), (pAddr, ch))

DECLARE_BANKED(vdp_cursor_addr, BANK(8), unsigned int, bk_conio_getvram, (), ())
DECLARE_BANKED_VOID(raw_cputc, BANK(8), bk_raw_cputc, (int ch), (ch))

int vdp_screenmode(int mode);
DECLARE_BANKED(vdp_screenmode, BANK(8), int, bk_vdp_screenmode, (int mode), (mode))

#define tputs_rom(x) bk_tputs_ram(str2ram(x))

#endif
