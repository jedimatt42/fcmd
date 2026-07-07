#include "banks.h"
#define MYBANK BANK(5)

#include "terminal.h"
#include "globals.h"

int handlePwd() {
  bk_tputs_ram(bk_uint2hex(currentDsr->crubase));
  bk_tputc('.');
  bk_tputs_ram(currentPath);
  bk_tputc('\n');
  return 0;
}
