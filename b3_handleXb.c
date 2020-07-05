#include "banks.h"
#define MYBANK BANK_3

#include "commands.h"
#include "b0_parsing.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include <conio.h>
#include <vdp.h>
#include <string.h>

extern void fg99();

extern char fg99_msg;

extern unsigned int fg99_addr;

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
    strcpy(namebuf, "PI");

    struct DeviceServiceRoutine *dsr = bk_findDsr(namebuf, 0);

    struct PAB pab;

    strcpy(namebuf, "PI.CONFIG");

    int err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_APPEND | DSR_TYPE_VARIABLE, 0);

    if (err) {
        tputs_rom("could no open PI.CONFIG\n");
        return;
    } else {
        unsigned char mapstring[12];
        strcpy(mapstring, "DSK1_DIR=FC");
        bk_dsr_write(dsr, &pab, mapstring, 11);
    }
    bk_dsr_close(dsr, &pab);

    // Write an XB program to TIPI.FC.FC/XB that contains:
    //   RUN "path-to-program argument"
    strcpy(namebuf, "TIPI");
    dsr = bk_findDsr(namebuf, 0);
    bk_initPab(&pab);
    strcpy(namebuf, "TIPI.FC.FC/XB");
    err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_OUTPUT | DSR_TYPE_VARIABLE, 80);
    if (err) {
        tputs_rom("failed to configure XB\n");
        return;
    } else {
        unsigned char line[81];
        strcpy(line, "10 RUN \"");
        strncpy(line+8, path, 72);
        strcpy(line+8+strlen(path), "\"");
        bk_dsr_write(dsr, &pab, line, strlen(line));
    }
    bk_dsr_close(dsr, &pab);

    // Launch XB
    strcpy(&fg99_msg, "TIXB_G");
    // 0x6372 - grom start address
    fg99_addr = 0x6372;

    lock_f18a();
    fg99();
}