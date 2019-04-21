#include "banks.h"

#define MYBANK BANK_4

#include "commands.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b4_labellist.h"
#include "b0_globals.h"

void handleGoto() {
  if (!scripton) {
    tputs("Error, 'goto' only valid in a script\n");
    return;
  }

  char* label = strtok(0, " ");
 
  if (!label) {
    tputs("Error, label required\n");
    return;
  } 

  int lineno = labels_get(label);
  if (-1 == lineno) {
    tputs("Error, no label named: ");
    tputs(label);
    tputs("\n");
    return;
  }

  // scriptPab should point to script name
  // reset back to first record
  bk_dsr_reset(scriptDsr, scriptPab, 0);
  // then script to line after the label we
  // are jumping to...
  for(int i=0; i<lineno; i++) {
    // reading sequential files with record
    // number 0 auto advances
    bk_dsr_read(scriptDsr, scriptPab, 0);
  }
}