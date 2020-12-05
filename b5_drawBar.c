#include "banks.h"
#define MYBANK BANK(5)

#include <conio.h>
#include "b8_terminal.h"
#include "b5_clock.h"
#include "b0_globals.h"
#include "b0_main.h"
#include "b10_detect_vdp.h"

#define HZ 0xCD
#define LB 0xB5
#define RB 0xC6
#define LE 0xD4
#define RE 0xBE

void drawBar() {
    int ox = conio_x;
    int oy = conio_y;
    gotoxy(0, 0);
    if (displayWidth == 80 && vdp_type == VDP_F18A) {
        conio_scrnCol = ((conio_scrnCol & 0x0f) << 4 ) | ((conio_scrnCol & 0xf0) >> 4);
    }
    bk_tputc(LE);
    bk_tputc(LB);
    if (displayWidth == 80) {
        tputs_rom("Force Command v");
    } else {
        bk_tputc('v');
    }
    tputs_rom(APP_VER);
    bk_tputc(RB);
    for (int i = conio_x; i < displayWidth - 11; i++) {
        bk_tputc(HZ);
    }
    bk_tputc(LB);
    struct DateTime dt;
    datetime(&dt);
    if (dt.hours < 10) {
        bk_tputc(HZ);
    }
    if (dt.year != dt.month) {
        gotoxy(displayWidth - 10, 0);
        pretty_time(&dt);
    }
    bk_tputc(RB);
    bk_tputc(RE);
    if (displayWidth == 80 && vdp_type == VDP_F18A) {
        conio_scrnCol = ((conio_scrnCol & 0x0f) << 4) | ((conio_scrnCol & 0xf0) >> 4);
    }
    gotoxy(ox, oy);
}