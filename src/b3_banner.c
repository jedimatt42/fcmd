#include "banks.h"
#define MYBANK BANK(3)

#include <conio.h>
#include "b8_terminal.h"
#include "string.h"

extern char bandata;

void banner() {
  char* bd = &bandata;
  while(*bd != 0) {
    bk_tputc(*bd++);
  }
  tputs_rom("\n");
  bk_bgcolor(4);
  bk_textcolor(15);
}

