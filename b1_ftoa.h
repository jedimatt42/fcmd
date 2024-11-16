#ifndef _FTOA_H
#define _FTOA_H 1

#include "banking.h"

void ftoa(char* dest, double a);

DECLARE_BANKED_VOID(ftoa, BANK(1), bk_ftoa, (char* dest, double a), (dest, a))

#endif
