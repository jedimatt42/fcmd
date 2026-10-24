#include "banks.h"
#define MYBANK BANK(9)

#include "redirect.h"
#include "globals.h"
#include "terminal.h"
#include <conio.h>
#include <vdp.h>
#include <string.h>

int redirect_finish(void) {
  int err = (int) redirect_state.error;

  if (redirect_state.open) {
    if (redirect_state.active && redirect_state.linelen > 0) {
      int ferr = bk_redirect_flush(0);
      if (err == 0) {
        err = ferr;
      }
    }

    unsigned int cerr = bk_dsr_close(redirect_state.dsr, &redirect_state.pab);
    if (err == 0) {
      err = (int) cerr;
    }
  }

  int last = nTextEnd - nTextRow;
  conio_x = redirect_state.saved_x;
  conio_y = redirect_state.saved_y;
  if (conio_x < 0) {
    conio_x = 0;
  } else if (conio_x > last) {
    conio_x = last;
  }
  if (conio_y < nTitleLine) {
    conio_y = nTitleLine;
  } else if (conio_y >= (int) displayHeight) {
    conio_y = (int) displayHeight - 1;
  }
  redirect_state.active = 0;
  redirect_state.open = 0;
  redirect_state.linelen = 0;
  redirect_state.last_cr = 0;
  redirect_state.esc_state = REDIR_ESC_NONE;

  if (err) {
    tputs_rom("error, redirect to ");
    bk_tputs_ram(redirect_state.path);
    tputs_rom(" failed (");
    bk_tputs_ram(bk_uint2hex((unsigned int) err));
    tputs_rom(")\n");
  } else if (redirect_state.empty_unsupported) {
    tputs_rom("note: blank lines were skipped; target does not accept empty records\n");
  }

  redirect_state.empty_unsupported = 0;
  return err;
}
