#include "banks.h"
#define MYBANK BANK(3)

#include "commands.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include <conio.h>
#include <string.h>

void handleTipiHalt() {
    char namebuf[14];
    bk_strcpy(namebuf, str2ram("PI"));

    struct DeviceServiceRoutine *dsr = bk_findDsr(namebuf, 0);

    struct PAB pab;

    bk_strcpy(namebuf, str2ram("PI.SHUTDOWN"));

    int err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_APPEND | DSR_TYPE_VARIABLE, 0);

    if (err)
    {
        tputs_rom("could no open PI.SHUTDOWN\n");
        return;
    }
    bk_dsr_close(dsr, &pab);

    tputs_rom("PI shutdown initiated, you must wait a bit before powering off.\n");
}