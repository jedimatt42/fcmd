#include <fc_api.h>
#include <kscan.h>
#include "menu.h"

int show_menu() {
  ui_drop_down(4);

  // I want fc_gotoxy(x, y);
  ui_gotoxy(3, 2);
  term_puts("^Q - close terminal");
  ui_gotoxy(3, 3);
  term_puts("BACK - resume");

  int done = 0;
  while(!done) {
    int key = term_kscan(5);

    if (KSCAN_STATUS & KSCAN_MASK) {
      switch(key) {
        case KEY_QUIT:
          return key;
        case KEY_BACK:
          return 0;
      }
    }
  }


  return KEY_QUIT;
}