#ifndef _setup_screen_h
#define _setup_screen_h 1

#include "banking.h"

void setupScreen(int width);
int isF18A();

DECLARE_BANKED_VOID(setupScreen, BANK(8), bk_setupScreen, (int width), (width))
DECLARE_BANKED(isF18A, BANK(8), int, bk_isF18A, (), ())

#endif
