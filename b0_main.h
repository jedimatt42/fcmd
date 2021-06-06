#ifndef _MAIN_H
#define _MAIN_H 1

#include "banking.h"
#include "b2_dsrutil.h"

#define APP_VER "1.20"

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

DECLARE_BANKED(runScript, BANK(0), int, bk_runScript, (struct DeviceServiceRoutine * dsr, char *scriptName), (dsr, scriptName))

#endif
