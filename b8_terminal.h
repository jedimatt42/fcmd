#ifndef _terminal_h
#define _terminal_h 1

#include "banking.h"
#include "b1_strutil.h"
// #include "conio.h"
unsigned int bgcolor(unsigned int color); // from conio
unsigned int textcolor(unsigned int color); // from conio
unsigned int bordercolor(unsigned int x); // from conio
unsigned int cgetc(); // from conio

void initTerminal();
void tputc(int c);
void tputs_ram(const char* str);
void enable_more();
void disable_more();

extern int termWidth;
extern unsigned char foreground;
extern unsigned char background;


DECLARE_BANKED_VOID(initTerminal, BANK(8), bk_initTerminal, (), ())
DECLARE_BANKED_VOID(tputc, BANK(8), bk_tputc, (int c), (c))
DECLARE_BANKED_VOID(tputs_ram, BANK(8), bk_tputs_ram, (const char* str), (str))
DECLARE_BANKED_VOID(enable_more, BANK(8), bk_enable_more, (), ())
DECLARE_BANKED_VOID(disable_more, BANK(8), bk_disable_more, (), ())

DECLARE_BANKED(bgcolor, BANK(8), unsigned int, bk_bgcolor, (unsigned int color), (color))
DECLARE_BANKED(textcolor, BANK(8), unsigned int, bk_textcolor, (unsigned int color), (color))
DECLARE_BANKED(bordercolor, BANK(8), unsigned int, bk_bordercolor, (unsigned int x), (x))
DECLARE_BANKED(cgetc, BANK(8), unsigned int, bk_cgetc, (), ())

#define tputs_rom(x)        \
{                           \
  unsigned char y[256];     \
  strncpy(y, x, 255);       \
  bk_tputs_ram(y);          \
}                           \

#endif