#ifndef _terminal_h
#define _terminal_h 1

#include "banking.h"
#include "b1cp_strutil.h"

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

#define tputs_rom(x)        \
{                           \
  unsigned char y[256];     \
  strncpy(y, x, 255);       \
  bk_tputs_ram(y);          \
}                           \

#endif