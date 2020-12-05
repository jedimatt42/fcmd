#include "banks.h"
#define MYBANK BANK(5)

#include <conio.h>
#include "b8_terminal.h"
#include "b5_clock.h"
#include "b0_globals.h"

void drawBar() {
    int ox = conio_x;
    int oy = conio_y;
    gotoxy(0, 0);
    bk_tputc(0xDB);
    bk_tputc(0xDD);
    tputs_rom("1.6");
    bk_tputc(0xDE);
    for (int i = conio_x; i < displayWidth - 9; i++) {
        bk_tputc(0xDB);
    }
    struct DateTime dt;
    datetime(&dt);
    if (dt.year != dt.month) {
        gotoxy(displayWidth - 10, 0);
        pretty_time(&dt);
    }
    gotoxy(ox, oy);
}