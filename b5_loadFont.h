#ifndef _LOADFONT_H
#define _LOADFONT_H 1

#include "banking.h"

int load_font();

DECLARE_BANKED(load_font, BANK(5), int, bk_load_font, (), ())

#endif
