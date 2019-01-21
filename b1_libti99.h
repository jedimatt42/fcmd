#ifndef _BANKED_LIBTI99_H
#define _BANKED_LIBTI99_H 1

#include <vdp.h>
#include <conio.h>
#include <string.h>

#include "banking.h"

DECLARE_BANKED(conio_getvram, BANK_1, unsigned int, bk_conio_getvram, (), ())
DECLARE_BANKED(bgcolor, BANK_1, unsigned int, bk_bgcolor, (unsigned int c), (c))
DECLARE_BANKED(bordercolor, BANK_1, unsigned int, bk_bordercolor, (unsigned int x), (x))
DECLARE_BANKED(textcolor, BANK_1, unsigned int, bk_textcolor, (unsigned int color), (color))
DECLARE_BANKED_VOID(cclear, BANK_1, bk_cclear, (unsigned int length), (length))
DECLARE_BANKED_VOID(cclearxy, BANK_1, bk_cclearxy, (int col, int row, int v), (col, row, v))
DECLARE_BANKED_VOID(chline, BANK_1, bk_chline, (int v), (v))
DECLARE_BANKED_VOID(chlinexy, BANK_1, bk_chlinexy, (int xx, int yy, int v), (xx, yy, v))
DECLARE_BANKED_VOID(clrscr, BANK_1, bk_clrscr, (), ())
DECLARE_BANKED_VOID(cputc, BANK_1, bk_cputc, (int ch), (ch))
DECLARE_BANKED_VOID(cputcxy, BANK_1, bk_cputcxy, (int xx, int yy, int ch), (xx, yy, ch))
DECLARE_BANKED_VOID(cputs, BANK_1, bk_cputs, (const char *s), (s))
DECLARE_BANKED_VOID(cputsxy, BANK_1, bk_cputsxy, (int xx, int yy, const char *s), (xx, yy, s))
DECLARE_BANKED_VOID(cvline, BANK_1, bk_cvline, (int len), (len))
DECLARE_BANKED_VOID(cvlinexy, BANK_1, bk_cvlinexy, (int x, int y, int len), (x, y, len))
DECLARE_BANKED_VOID(screensize, BANK_1, bk_screensize, (unsigned char *x, unsigned char *y), (x, y))
DECLARE_BANKED_CHAR(kbhit, BANK_1, unsigned char, bk_kbhit, (), ())
DECLARE_BANKED_CHAR(cgetc, BANK_1, unsigned char, bk_cgetc, (), ())
DECLARE_BANKED_CHAR(reverse, BANK_1, unsigned char, bk_reverse, (unsigned char x), (x))
DECLARE_BANKED_CHAR(vdpreadchar, BANK_1, unsigned char, bk_vdpreadchar, (int pAddr), (pAddr))

DECLARE_BANKED(strlen, BANK_1, int, bk_strlen, (const char *s), (s))
DECLARE_BANKED(atoi, BANK_1, int, bk_atoi, (char *s), (s))
DECLARE_BANKED(strcpy, BANK_1, char*, bk_strcpy, (char *d, const char *s), (d, s))
DECLARE_BANKED(uint2str, BANK_1, char *, bk_uint2str, (unsigned int x), (x))
DECLARE_BANKED(int2str, BANK_1, char *, bk_int2str, (int x), (x))
DECLARE_BANKED(uint2hex, BANK_1, char*, bk_uint2hex, (unsigned int x), (x))

DECLARE_BANKED_VOID(set_text80_color, BANK_1, bk_set_text80_color, (), ())
DECLARE_BANKED_VOID(set_text, BANK_1, bk_set_text, (), ())
DECLARE_BANKED_VOID(vdpmemread, BANK_1, bk_vdpmemread, (int pAddr, unsigned char *pDest, int cnt), (pAddr, pDest, cnt))
DECLARE_BANKED_VOID(vdpmemcpy, BANK_1, bk_vdpmemcpy, (int pAddr, const unsigned char *pSrc, int cnt), (pAddr, pSrc, cnt))
DECLARE_BANKED_VOID(vdpmemset, BANK_1, bk_vdpmemset, (int pAddr, int ch, int cnt), (pAddr, ch, cnt))
DECLARE_BANKED_VOID(unlock_f18a, BANK_1, bk_unlock_f18a, (), ())
DECLARE_BANKED_VOID(lock_f18a, BANK_1, bk_lock_f18a, (), ())
DECLARE_BANKED_VOID(set_graphics, BANK_1, bk_set_graphics, (int sprite_mode), (sprite_mode))
DECLARE_BANKED_VOID(charsetlc, BANK_1, bk_charsetlc, (), ())

#endif