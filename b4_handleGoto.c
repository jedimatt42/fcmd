#include "banks.h"

#define MYBANK BANK(4)

#include "commands.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b4_labellist.h"
#include "b0_globals.h"
#include <vdp.h>
#include <string.h>

void handleGoto() {
  if (!scripton) {
    tputs_rom("Error, 'goto' only valid in a script\n");
    return;
  }

  char* label = bk_strtok(0, ' ');

  if (!label) {
    tputs_rom("Error, label required\n");
    return;
  }

  int gotoline = labels_get(label);
  if (-1 == gotoline) {
    char commandbuf[255];
    // scan forward for label definitions :(

    int record = *goto_line_ref;
    while((bk_dsr_status(scriptDsr, scriptPab) & DSR_STATUS) != DSR_STATUS_EOF) {
      int ferr = bk_dsr_read(scriptDsr, scriptPab, 0);
      record++;
      if (!ferr) {
        bk_strset(commandbuf, 0, 255);
        vdpmemread(scriptPab->VDPBuffer, commandbuf, scriptPab->CharCount);
        char* tok = bk_strtok(commandbuf, ' ');
        if (tok[bk_strlen(tok)-1] == ':') {
          tok[bk_strlen(tok)-1] = 0; // shorten to just the name
          labels_add(tok, record);
        }
      }
    }
    gotoline = labels_get(label);
  }

  if (-1 == gotoline) {
    tputs_rom("Error, no label named: ");
    bk_tputs_ram(label);
    bk_tputc('\n');
    gotoline = *goto_line_ref;
  } else {
    *goto_line_ref = gotoline;
  }
  // either return to where we left off, or advance to the target goto line

  // scriptPab should point to script name
  // reset back to first record
  bk_dsr_reset(scriptDsr, scriptPab, 0);
  // then script to line after the label we
  // are jumping to...
  for(int i=0; i<gotoline; i++) {
    // reading sequential files with record
    // number 0 auto advances
    bk_dsr_read(scriptDsr, scriptPab, 0);
  }
}
