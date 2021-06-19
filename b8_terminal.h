#ifndef _terminal_h
#define _terminal_h 1

#include "banking.h"
#include "b1_strutil.h"

unsigned int bgcolor(unsigned int color); // from conio
unsigned int textcolor(unsigned int color); // from conio
unsigned int bordercolor(unsigned int x); // from conio
unsigned int kscan(unsigned int mode); // from kscan
void gplvdp(int vect, int adr, int cnt); // from vdp_gplvdp
void vdp_setchar(int pAddr, int ch); // write a char to screen address
unsigned int conio_getvram(); // returns VDP address of cursor

typedef void (*identify_callback)(int flag);

void initTerminal();
void set_identify_hook(identify_callback cb);
void tputc(int c);
void tputs_ram(const char* str);

extern unsigned char foreground;
extern unsigned char background;


DECLARE_BANKED_VOID(initTerminal, BANK(8), bk_initTerminal, (), ())
DECLARE_BANKED_VOID(set_identify_hook, BANK(8), bk_set_identify_hook, (identify_callback cb), (cb))
DECLARE_BANKED_VOID(tputc, BANK(8), bk_tputc, (int c), (c))
DECLARE_BANKED_VOID(tputs_ram, BANK(8), bk_tputs_ram, (const char* str), (str))
DECLARE_BANKED_VOID(gplvdp, BANK(8), bk_gplvdp, (int vect, int adr, int cnt), (vect, adr, cnt))

DECLARE_BANKED(bgcolor, BANK(8), unsigned int, bk_bgcolor, (unsigned int color), (color))
DECLARE_BANKED(textcolor, BANK(8), unsigned int, bk_textcolor, (unsigned int color), (color))
DECLARE_BANKED(bordercolor, BANK(8), unsigned int, bk_bordercolor, (unsigned int x), (x))
DECLARE_BANKED(kscan, BANK(8), unsigned int, bk_kscan, (unsigned int mode), (mode))
DECLARE_BANKED_VOID(vdp_setchar, BANK(8), bk_vdp_setchar, (int pAddr, int ch), (pAddr, ch))

DECLARE_BANKED(conio_getvram, BANK(8), unsigned int, bk_conio_getvram, (), ())

#define tputs_rom(x) bk_tputs_ram(str2ram(x))

#endif