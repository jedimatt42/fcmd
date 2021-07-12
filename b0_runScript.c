#include "banks.h"
#define MYBANK BANK(0)

#include <vdp.h>
#include <conio.h>
#include <kscan.h>

#include "b0_main.h"
#include "b8_terminal.h"
#include "b10_parsing.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b4_labellist.h"


int runScript(struct DeviceServiceRoutine* dsr, char* scriptName) {
  int ran = 0;
  struct DeviceServiceRoutine* oldDsr = scriptDsr;
  scriptDsr = dsr;
  struct PAB pab;
  scriptPab = &pab;
  bk_labels_clear();
  scripton++;
  int lineno = 0;

  int ferr = bk_dsr_open(dsr, &pab, scriptName, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL, 0);
  if (!ferr) {
    ran = 1;
    while(!ferr) {
      checkBackspaceVar();
      VDP_INT_POLL;
      char commandbuf[256];

      bk_strset(commandbuf, 0, 255);
      ferr = bk_dsr_read(dsr, &pab, 0);
      char k = bk_kscan(5);
      if (k == 131 || k == 2) { // control-c or alt-4
        ferr = 1; // so abort script
      }
      if (!ferr) {
        lineno++;
        vdpmemread(pab.VDPBuffer, commandbuf, pab.CharCount);
        int l = bk_strlen(commandbuf);
        // TI-Writer adds \r to lines, so erase those automatically if at end of line.
        if (commandbuf[l-1] == 13) {
          commandbuf[l-1] = 0;
        }
        // This is a bit brutish, but keeps from having to pass state around
        // close the script before invoking a command
        volatile int must_close = bk_must_close_command(commandbuf);
        if (must_close) {
          bk_dsr_close(dsr, &pab);
        }
        goto_line_ref = &lineno;
        bk_handleCommand(commandbuf);
        if (must_close) {
          // now re-open and seek in the script to the next line.
          ferr = bk_dsr_open(dsr, &pab, scriptName, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL, 0);
          // advance to next line
          int newline = 0;
          while (!ferr && newline < lineno) {
            ferr = bk_dsr_read(dsr, &pab, 0);
            newline++;
          }
        }
      }
    }
    bk_dsr_close(dsr, &pab);
  }
  scripton--;
  return ran;
}
