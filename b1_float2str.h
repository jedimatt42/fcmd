#ifndef _FLOAT2STR_H
#define _FLOAT2STR_H 1

#include "banking.h"

char* float2str(double a);

DECLARE_BANKED(float2str, BANK(1), char*, bk_float2str, (double a), (a))

#endif
