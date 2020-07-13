#include "banks.h"
#define MYBANK BANK(0)

#include "commands.h"
#include "b0_main.h"
#include "b2_dsrutil.h"
#include "b0_parsing.h"
#include "b1cp_terminal.h"

void handleCall() {
  struct DeviceServiceRoutine* dsr;
  char path[256];
  bk_parsePathParam(&dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    tputs_rom("no file specified\n");
    return;
  }

  int ran = runScript(dsr, path);
  if (!ran) {
    tputs_rom("error, file not found: ");
    tputs_ram(path);
    tputc('\n');
  }
}