#ifndef _GETSTR_H
#define _GETSTR_H 1

#include "banking.h"

extern int history_on;

// work with the conio library for complex keyboard input.
void getstr(char* var, int limit, int backspace);

unsigned int cgetc(unsigned int cursor);
#define CUR_OVERWRITE 219
#define CUR_INSERT '_'

void history_mode(int enable);

DECLARE_BANKED_VOID(getstr, BANK(8), bk_getstr, (char* var, int limit, int backspace), (var, limit, backspace))
DECLARE_BANKED(cgetc, BANK(8), unsigned int, bk_cgetc, (unsigned int cursor), (cursor))

#endif