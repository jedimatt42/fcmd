#ifndef _MAIN_H
#define _MAIN_H 1

void initGlobals();
void main();
void resetF18A();
void setupScreen(int width);
void sleep(int jiffies);
void titleScreen();

#include "banking.h"

DECLARE_BANKED_VOID(resetF18A, BANK_0, bk_resetF18A, (), ())

#endif