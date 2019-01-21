#include "banks.h"

#define MYBANK BANK_2

#include "b2_commands.h"
#include "b1_libti99.h"
#include "conio.h"

void handleCls() {
  bk_clrscr();
  gotoxy(0,0);
}

