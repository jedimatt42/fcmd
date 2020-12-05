#include "banks.h"

#define MYBANK BANK(5)

#include "b5_clock.h"
#include "b8_terminal.h"
#include "b1_strutil.h"

void handleDate() {
    struct DateTime dt;
    bk_datetime(&dt);

    bk_tputs_ram(bk_uint2str(dt.month));
    bk_tputc('-');
    bk_tputs_ram(bk_uint2str(dt.day));
    bk_tputc('-');
    bk_tputs_ram(bk_uint2str(dt.year));
    bk_tputc(' ');

    pretty_time(&dt);

    bk_tputc('\n');
}