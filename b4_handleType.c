#include "banks.h"
#define MYBANK BANK_4

#include <string.h>
#include "commands.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b2_dsrutil.h"
#include "b0_parsing.h"
#include <vdp.h>

void handleType() {
  char namebuf[256];
  struct DeviceServiceRoutine* dsr;
  bk_parsePathParam(&dsr, namebuf, PR_REQUIRED);
  if (dsr == 0) {
    tputs("error, no file specified\n");
    return;
  }

  struct PAB pab;

  int err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL, 0);
  
  if (err) {
    tputs("could not open ");
    tputs(namebuf);
    tputc('\n');
    return;
  }
  while(!err) {
    err = bk_dsr_read(dsr, &pab, 0);
    if (!err) {
      // print the line...
      char linebuf[256];
      vdpmemread(pab.VDPBuffer, linebuf, pab.CharCount);
      linebuf[pab.CharCount] = 0;

      tputs(linebuf);
      tputc('\n');
    }
  }
  bk_dsr_close(dsr, &pab);
}