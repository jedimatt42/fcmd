#ifndef _palette_h
#define _palette_h 1

#include "banking.h"

void set_palette(int palset, const char* palette);

DECLARE_BANKED_VOID(set_palette, BANK(7), bk_set_palette, (int palset, const char* palette), (palset, palette))

#endif
