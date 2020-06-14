#include "banks.h"
#define MYBANK BANK_4

#include <string.h>
#include "commands.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b2_dsrutil.h"
#include "b0_parsing.h"
#include <vdp.h>
#include <kscan.h>

#define CTRLZ 0x001A

void handleType() {
  int ansi = 0;

  char* tok = strtokpeek(0, " ");
  if (!strcmpi("/ansi", tok)) {
    ansi = 1;
    strtok(0, " "); // consume optional token
  }

  char namebuf[256];
  struct DeviceServiceRoutine* dsr;
  bk_parsePathParam(&dsr, namebuf, PR_REQUIRED);
  if (dsr == 0) {
    tputs_rom("error, no file specified\n");
    return;
  }

  struct PAB pab;

  int flags = DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_SEQUENTIAL;
  if (ansi) {
    // bitwise or of zero: flags |= DSR_TYPE_FIXED;
    disable_more();
  } else {
    flags |= DSR_TYPE_VARIABLE;
  }

  int err = bk_dsr_open(dsr, &pab, namebuf, flags, 0);

  if (err) {
    tputs_rom("could not open ");
    tputs_ram(namebuf);
    tputc('\n');
    return;
  }
  int rec = 0;
  while(!err) {
    char k = kscan(5);
    if (k == 131 || k == 2) { // control-c or alt-4
      break;
    }

    char linebuf[256];

    err = bk_dsr_read(dsr, &pab, rec++);
    if (!err) {
      // print the line...
      vdpmemread(pab.VDPBuffer, linebuf, pab.CharCount);

      int i = 0;
      while(i<pab.CharCount) {
        if (ansi) {
          if (linebuf[i] == CTRLZ) {
            err = 1;
            break;
          }
        }
        tputc(linebuf[i]);
        i++;
      }

      if (!ansi) {
        if (i==0 || i!=80) {
          tputc('\n');
        }
      }
    }
  }
  bk_dsr_close(dsr, &pab);
}