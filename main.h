#ifndef _MAIN_H
#define _MAIN_H 1

#include "dsrutil.h"

void initGlobals();
void main();
void resetF18A();
void setupScreen(int width);
void sleep(int jiffies);
void titleScreen();
void onVolInfo(struct VolInfo* volInfo);
void onDirEntry(struct DirEntry* dirEntry);

#endif