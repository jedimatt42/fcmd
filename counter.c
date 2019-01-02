#include <vdp.h>
#include <conio.h>

#include "counter.h"

void setupScreen() {
  set_text();
  charset();
  textcolor(COLOR_BLACK);
  clrscr();
  gotoxy(0,0);
  cputs("Hello World!");
}

int countup(int x, int y, int z) {
  return x + y * z;
}

void checkresult(int s) {
  gotoxy(0,1);
  if (s == 9) {
    cputs("countup - good");
  } else {
    cputs("countup - error");
  }
}

