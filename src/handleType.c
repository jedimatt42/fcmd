#include "banks.h"
#define MYBANK BANK(4)

#include "commands.h"
#include "globals.h"
#include "strutil.h"
#include "terminal.h"
#include "dsrutil.h"
#include "parsing.h"
#include <vdp.h>
#include <kscan.h>

#define CTRLZ 0x001A

int handleType() {
  int ansi = 0;

  char* tok = bk_strtokpeek(0, ' ');
  if (!bk_strcmpi(str2ram("/ansi"), tok)) {
    ansi = 1;
    bk_strtok(0, ' '); // consume optional token
  }

  char namebuf[256];
  struct DeviceServiceRoutine* dsr;
  bk_parsePathParam(0, &dsr, namebuf, PR_REQUIRED);
  if (dsr == 0) {
    tputs_rom("error, no file specified\n");
  return 0;
  }

  struct PAB pab;

  int flags = DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_SEQUENTIAL;
  if (!ansi) {
    flags |= DSR_TYPE_VARIABLE;
  }

  int err = bk_dsr_open(dsr, &pab, namebuf, flags, 0);

  if (err) {
    tputs_rom("could not open ");
    bk_tputs_ram(namebuf);
    bk_tputc('\n');
  return 0;
  }
  int rec = 0;
  char linebuf[256];
  while(!err && request_break == 0) {
    err = bk_dsr_read_cpu(dsr, &pab, rec++, linebuf);
    if (!err) {
      int i = 0;
      while(i<pab.CharCount) {
        if (ansi) {
          if (linebuf[i] == CTRLZ) {
            err = 1;
            break;
          }
        }
        bk_tputc(linebuf[i]);
        i++;
      }

      if (!ansi) {
        if (i==0 || i!=80) {
          bk_tputc('\n');
        }
      }
    }
  }
  bk_dsr_close(dsr, &pab);
  return 0;
}