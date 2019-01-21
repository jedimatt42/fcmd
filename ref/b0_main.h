#ifndef _MAIN_H
#define _MAIN_H 1

#include "b2_dsrutil.h"

void initGlobals();
void main();
void resetF18A();
void setupScreen(int width);
void sleep(int jiffies);
void titleScreen();
void onVolInfo(struct VolInfo* volInfo);
void onDirEntry(struct DirEntry* dirEntry);

#include "banking.h"

DECLARE_BANKED_VOID(setupScreen, BANK_0, bk_setupScreen, (int width), ())
DECLARE_BANKED_VOID(sleep, BANK_0, bk_sleep, (), ())
DECLARE_BANKED_VOID(titleScreen, BANK_0, bk_titleScreen, (), ())

#endif