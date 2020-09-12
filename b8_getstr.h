#ifndef _GETSTR_H
#define _GETSTR_H 1

#include "banking.h"

// work with the conio library for complex keyboard input.
void getstr(char* var, int limit, int backspace);

DECLARE_BANKED_VOID(getstr, BANK(8), bk_getstr, (char* var, int limit, int backspace), (var, limit, backspace))

#endif