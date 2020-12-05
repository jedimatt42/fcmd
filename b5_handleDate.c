#include "banks.h"

#define MYBANK BANK(5)

#include "b5_clock.h"
#include "b8_terminal.h"
#include "b1_strutil.h"

void handleDate() {
    struct DateTime dt;
    bk_datetime(&dt);
    int pm = 0;

    bk_tputs_ram(bk_uint2str(dt.month));
    bk_tputc('-');
    bk_tputs_ram(bk_uint2str(dt.day));
    bk_tputc('-');
    bk_tputs_ram(bk_uint2str(dt.year));
    bk_tputc(' ');
    if (dt.hours > 12) {
        pm = 1;
        dt.hours -= 12;
    }
    bk_tputs_ram(bk_uint2str(dt.hours));
    bk_tputc(':');
    bk_tputs_ram(bk_uint2str(dt.minutes));
    if (pm) {
        tputs_rom("pm");
    } else {
        tputs_rom("am");
    }
    bk_tputc('\n');
}