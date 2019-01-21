#ifndef _GETSTR_H
#define _GETSTR_H 1

// work with the conio library for complex keyboard input.
void getstr(int x, int y, char* var, int limit, int backspace);

#include "banking.h"

DECLARE_BANKED_VOID(getstr, BANK_1, bk_getstr, (int x, int y, char* var, int limit, int backspace), (x, y, var, limit, backspace))

#endif