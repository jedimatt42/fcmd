#ifndef _MAIN_H
#define _MAIN_H 1

#include "b2_dsrutil.h"

#define APP_VER "1.0"

void initGlobals();
void main();
void sleep(int jiffies);
void titleScreen();
void playtipi();
int runScript(struct DeviceServiceRoutine* dsr, char* scriptName);

inline void reboot() {
  __asm__(
    "blwp @>0000"
  );
}

#endif
