#ifndef _HEAP_H
#define _HEAP_H 1

#include "banking.h"

char* alloc(int sz);
void free(char* addr);

extern char* heap_end;

DECLARE_BANKED(alloc, BANK(0), char*, bk_alloc, (int sz), (sz))
DECLARE_BANKED_VOID(free, BANK(0), bk_free, (char* addr), (addr))

#endif