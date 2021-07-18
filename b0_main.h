#ifndef _MAIN_H
#define _MAIN_H 1


#define APP_VER "1.25"

void initGlobals();
void main();
void sleep(int jiffies);
void titleScreen();
void playtipi();
void checkBackspaceVar();

inline void reboot() {
  __asm__(
    "blwp @>0000"
  );
}

#endif
