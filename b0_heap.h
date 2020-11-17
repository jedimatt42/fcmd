#ifndef _HEAP_H
#define _HEAP_H 1

#include "banking.h"

char* alloc(int sz);

extern char* heap_end;

DECLARE_BANKED(alloc, BANK(0), char*, bk_alloc, (int sz), (sz))

#endif