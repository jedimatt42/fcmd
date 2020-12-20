#include "banks.h"
#define MYBANK BANK(5)

#include "b8_terminal.h"
#include "b0_globals.h"

void handlePwd() {
  bk_tputs_ram(bk_uint2hex(currentDsr->crubase));
  bk_tputc('.');
  bk_tputs_ram(currentPath);
  bk_tputc('\n');
}
