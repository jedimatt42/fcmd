#ifndef _FLOAT2STR_H
#define _FLOAT2STR_H 1

#include "banking.h"

char* str_from_float(double a);

DECLARE_BANKED(str_from_float, BANK(1), char*, bk_float2str, (double a), (a))

#endif
