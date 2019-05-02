#ifndef _GETSTR_H
#define _GETSTR_H 1

#include "banking.h"

// work with the conio library for complex keyboard input.
void getstr(int x, int y, char* var, int limit, int backspace);

DECLARE_BANKED_VOID(getstr, BANK_0, bk_getstr, (int x, int y, char* var, int limit, int backspace), (x, y, var, limit, backspace))

#endif