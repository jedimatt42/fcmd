#include "banks.h"

#define MYBANK BANK(1)

#include "commands.h"
#include "history.h"
#include "sams.h"
#include "terminal.h"
#include "globals.h"

int handleHistory() {
  char buffer[256];
  if (sams_total_pages) {
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
  } else {
    // vdp type single entry history
    history_redo(buffer, 256, HIST_GET);
    tputs_rom("\n1: ");
    bk_tputs_ram(buffer);
    bk_tputc('\n');
  }
  return 0;
}