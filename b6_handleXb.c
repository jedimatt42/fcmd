#include "banks.h"
#define MYBANK BANK(6)

#include "commands.h"
#include "b0_parsing.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b8_setupScreen.h"
#include "b4_variables.h"
#include "b1_fg99.h"
#include <conio.h>
#include <vdp.h>
#include <string.h>

void handleXb() {
    struct DeviceServiceRoutine *xbdsr;
    char path[256];
    bk_parsePathParam(&xbdsr, path, PR_REQUIRED);
    if (xbdsr == 0) {
        tputs_rom("no XB program specified\n");
        return;
    }

    // MAP DSK1 to TIPI.FC
    char namebuf[14];
    bk_strcpy(namebuf, str2ram("PI"));

    struct DeviceServiceRoutine *dsr = bk_findDsr(namebuf, 0);

    struct PAB pab;

    bk_strcpy(namebuf, str2ram("PI.CONFIG"));

    int err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_APPEND | DSR_TYPE_VARIABLE, 0);

    if (err) {
        tputs_rom("could no open PI.CONFIG\n");
        return;
    } else {
        unsigned char mapstring[12];
        bk_strcpy(mapstring, str2ram("DSK1_DIR=FC"));
        bk_dsr_write(dsr, &pab, mapstring, 11);
    }
    bk_dsr_close(dsr, &pab);

    // Write an XB program to TIPI.FC.FC/XB that contains:
    //   RUN "path-to-program argument"
    bk_strcpy(namebuf, str2ram("TIPI"));
    dsr = bk_findDsr(namebuf, 0);
    bk_initPab(&pab);
    bk_strcpy(namebuf, str2ram("TIPI.FC.FC/XB"));
    err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_OUTPUT | DSR_TYPE_VARIABLE, 80);
    if (err) {
        tputs_rom("failed to configure XB\n");
        return;
    } else {
        unsigned char line[81];
        bk_strcpy(line, str2ram("10 RUN \""));
        bk_strncpy(line+8, path, 72);
        bk_strcpy(line+8+bk_strlen(path), str2ram("\""));
        bk_dsr_write(dsr, &pab, line, bk_strlen(line));
    }
    bk_dsr_close(dsr, &pab);

    // Launch XB
    char fg99_msg[20];
    fg99_msg[0] = 0x99;
    bk_strcpy(fg99_msg + 1, str2ram("OKFG99"));
    fg99_msg[7] = 0x99;
    bk_strcpy(namebuf, str2ram("XBMOD"));
    char* xb_module = bk_vars_get(namebuf);
    if (-1 == (int)xb_module) {
        bk_strcpy(fg99_msg+8, str2ram("TIXB_G"));
    } else {
        bk_strcpy(fg99_msg+8, xb_module);
    }
    for (int i = 16; i < 20; i++) {
        fg99_msg[i] = 0x00;
    }

    // 0x6372 - grom start address of standard TI XB version 110
    bk_strcpy(namebuf, str2ram("XBADDR"));
    char* xb_addr = bk_vars_get(namebuf);

    int fg99_addr = 0;
    if (-1 == (int)xb_addr) {
        fg99_addr = 0x6372;
    } else {
        fg99_addr = bk_atoi(xb_addr);
    }

    bk_setupScreen(0);
    bk_fg99(fg99_msg, fg99_addr);
}