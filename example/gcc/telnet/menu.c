#include <fc_api.h>
#include <kscan.h>
#include "menu.h"

int show_menu() {
  fc_ui_drop_down(4);

  // I want fc_gotoxy(x, y);
  fc_ui_gotoxy(3, 2);
  fc_tputs("^Q - close terminal");
  fc_ui_gotoxy(3, 3);
  fc_tputs("BACK - resume");

  int done = 0;
  while(!done) {
    int key = fc_kscan(5);

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