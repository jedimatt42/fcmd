#include "banks.h"

#define MYBANK BANK(1)

#include "commands.h"
#include "b1_history.h"
#include "b0_sams.h"
#include "b8_terminal.h"
#include "b0_globals.h"

void handleHistory() {
  char buffer[256];
  if (sams_total_pages) {
    HIST_IDX = 0;
    unsigned int cnt = 1;
    history_redo(buffer, 256, HIST_GET);
    bk_tputc('\n');
    while(bk_strlen(buffer) && request_break == 0) {
      bk_tputs_ram(uint2str(cnt));
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
}