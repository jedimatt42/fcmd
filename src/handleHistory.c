#include "banks.h"

#define MYBANK BANK(1)

#include "commands.h"
#include "history.h"
#include "terminal.h"
#include "globals.h"

int handleHistory() {
  char buffer[256];
  HIST_IDX = 0;
  unsigned int cnt = 1;
  history_redo(buffer, 256, HIST_GET);
  bk_tputc('\n');
  while(bk_strlen(buffer) && request_break == 0) {
    bk_tputs_ram(str_from_uint(cnt));
    bk_tputc(':');
    bk_tputc(' ');
    bk_tputs_ram(buffer);
    bk_tputc('\n');
    cnt++;
    history_redo(buffer, 256, HIST_GET);
  }
  HIST_IDX = 0;
  return 0;
}
