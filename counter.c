#include <vdp.h>
#include <conio.h>

#include "counter.h"
#include "main.h"
#include "banking.h"

#define MYBANK BANK_1

DECLARE_BANKED(test2, BANK_0, void far_test2(), trampoline())

void setupScreen() {
  set_text();
  charset();
  textcolor(COLOR_BLACK);
  clrscr();
  gotoxy(0,0);
  cputs("Bank >6002: Hello World!");
}

int countup(int x, int y, int z) {
  return x + y + z;
}

void checkresult(int s, int ex) {
  gotoxy(0,1);
  if (s == ex) {
    cputs("Bank >6002: countup - good");
  } else {
    cputs("Bank >6002: countup - error");
  }
  gotoxy(0,3);
}

void test1() {
  cputc('b');
  far_test2();
  cputc('B');
}
