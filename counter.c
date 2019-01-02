#include "counter.h"
#include <vdp.h>
#include <conio.h>

void setupScreen() {
  set_text();
  charset();
  textcolor(COLOR_BLACK);
  clrscr();
  gotoxy(0,0);
  cputs("Hello World!");
  gotoxy(1,0);
}

int countup(int x, int y, int z) {
  return x + y * z;
}

