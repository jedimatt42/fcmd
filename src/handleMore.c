#include "banks.h"
#define MYBANK BANK(2)

#include "commands.h"
#include "globals.h"
#include "strutil.h"
#include "terminal.h"
#include "dsrutil.h"
#include "parsing.h"
#include "variables.h"
#include "getstr.h"
#include "redirect.h"
#include <conio.h>
#include <vdp.h>

#define KEY_BREAK 2
#define KEY_BREAK_ALT 131

/*
 * Rows used by one DISPLAY/VARIABLE record, including terminal wrapping and
 * the newline that 'type' style output adds after records shorter than 80.
 */
static int record_rows(int len) {
  int width = (int) displayWidth;

  if (len == 80) {
    return (80 + width - 1) / width;
  }
  return (len / width) + 1;
}

static void clear_prompt_line(void) {
  bk_cursorGoto(1, (int) displayHeight);
  for (int i = 1; i < (int) displayWidth; i++) {
    bk_tputc(' ');
  }
  bk_cursorGoto(1, (int) displayHeight);
}

/*
 * Wait at the bottom of the screen. Any key except Q continues; Q, FCTN-4, or
 * CTRL-C stops paging. Returns 1 when the user wants to stop.
 */
static int more_prompt(void) {
  bk_cursorGoto(1, (int) displayHeight);
  tputs_rom("-- More --");

  unsigned int key = bk_cgetc(CUR_INSERT);
  if (key == KEY_BREAK || key == KEY_BREAK_ALT) {
    request_break = 1;
    clear_prompt_line();
    return 1;
  }
  if (key == 'q' || key == 'Q') {
    clear_prompt_line();
    return 1;
  }

  return 0;
}

int handleMore() {
  struct DeviceServiceRoutine* dsr;
  char namebuf[256];

  char* peek = bk_strtokpeek(0, ' ');
  if (peek == 0) {
    /* No explicit path: consume the previous pipeline stage, if any. */
    char pipein[MAX_VAR_VAL + 1];
    if (!bk_vars_get(str2ram("PIPEIN"), pipein, sizeof(pipein)) || pipein[0] == 0) {
      tputs_rom("error, no file specified and PIPEIN is empty\n");
      return 1;
    }
    bk_parsePathParam(pipein, &dsr, namebuf, PR_REQUIRED);
  } else {
    bk_parsePathParam(0, &dsr, namebuf, PR_REQUIRED);
  }

  if (dsr == 0) {
    tputs_rom("error, no file specified\n");
    return 1;
  }

  struct PAB pab;
  int flags = DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_SEQUENTIAL | DSR_TYPE_VARIABLE;
  int err = bk_dsr_open(dsr, &pab, namebuf, flags, 0);
  if (err) {
    tputs_rom("could not open ");
    bk_tputs_ram(namebuf);
    bk_tputc('\n');
    return 1;
  }

  /*
   * While redirected, paging prompts would be captured along with the text.
   * Act like 'type' instead so the final pipeline stage or > target gets the
   * complete contents.
   */
  int paging = !redirect_state.active;
  int page_rows = (int) displayHeight - nTitleLine - 1;
  if (page_rows < 1) {
    page_rows = 1;
  }

  int rows_left = (int) displayHeight - 1 - (int) conio_y;
  if (paging && rows_left < 1) {
    bk_term_cls();
    rows_left = page_rows;
  }

  int failed = 0;
  int rec = 0;
  char linebuf[256];

  while (!request_break) {
    err = bk_dsr_read_cpu(dsr, &pab, rec++, linebuf);
    if (err) {
      if (err != DSR_ERR_PASTEOF) {
        failed = 1;
      }
      break;
    }

    int len = pab.CharCount;
    int rows = record_rows(len);

    /* Read ahead first, so a file ending exactly at the page boundary does
       not prompt for an empty next page. */
    if (paging && rows_left < rows) {
      if (more_prompt()) {
        break;
      }
      bk_term_cls();
      rows_left = page_rows;
    }

    for (int i = 0; i < len; i++) {
      bk_tputc(linebuf[i]);
    }
    if (len != 80) {
      bk_tputc('\n');
    }

    if (paging) {
      rows_left -= rows;
    }
  }

  bk_dsr_close(dsr, &pab);
  return failed;
}
