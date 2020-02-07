#include "banks.h"
#define MYBANK BANK_0

#include "commands.h"
#include "b0_main.h"
#include "b2_dsrutil.h"
#include "b0_parsing.h"
#include "b1cp_terminal.h"

void handleCall() {
  struct DeviceServiceRoutine* dsr;
  char filename[60];
  parsePathParam(&dsr, filename, PR_REQUIRED);

  int ran = runScript(dsr, filename);
  if (!ran) {
    tputs_rom("error, file not found: ");
    tputs_ram(filename);
    tputc('\n');
  }
}