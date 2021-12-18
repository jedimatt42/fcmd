#include "banks.h"
#define MYBANK BANK(13)

#include "b0_globals.h"
#include "b10_parsing.h"
#include "b0_sams.h"
#include "b2_dsrutil.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b8_getstr.h"
#include "b13_honk.h"
#include "vdp.h"
#include "conio.h"
#include "b1_history.h"
#include "b13_editor.h"


int commandLineEd(char* cmdline, int limit) {
  // command line editor using same code as full screen editor - mostly
  if (sams_total_pages) {
    int pagebase = bk_alloc_pages(1);
    bk_map_page(pagebase, 0xA000);
  }

  EDIT_BUFFER->justRendered = 0;
  EDIT_BUFFER->ed_mode = ED_LINE;
  EDIT_BUFFER->max_lines = 1;
  EDIT_BUFFER->lineCount = 1;
  EDIT_BUFFER->screen_x = conio_x;
  EDIT_BUFFER->screen_y = conio_y;
  EDIT_BUFFER->offset_x = 0;
  EDIT_BUFFER->offset_y = 0;

  ed_clearBuffer();

  ed_renderLines();

  conio_x = EDIT_BUFFER->screen_x;
  conio_y = EDIT_BUFFER->screen_y;

  int quit = ed_edit_loop(0); // null devpath as save is disabled.

  if (quit == 1) {
    bk_strncpy(cmdline, EDIT_BUFFER->lines[0].data, limit);
  }

  if (sams_total_pages) {
    bk_free_pages(1);
  }

  return 0; // cmdline is left empty on ctrl-c|break
}

void handleEd() {
  struct DeviceServiceRoutine* dsr = 0;
  char devpath[80];
  char* path = devpath + 5;
  bk_parsePathParam(0, &dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    tputs_rom("no file specified\n");
    return;
  }

  if (sams_total_pages) {
    int pagebase = bk_alloc_pages(6);
    for(int i=pagebase; i<pagebase+6; i++) {
      bk_map_page(i, 0xA000 + ((i-pagebase) * 0x1000));
    }
  }

  // A tiny DV80 editor
  EDIT_BUFFER->justRendered = 0;
  EDIT_BUFFER->ed_mode = ED_FULL;
  EDIT_BUFFER->max_lines = MAX_LINES;
  EDIT_BUFFER->lineCount = 1;
  EDIT_BUFFER->screen_x = 0;
  EDIT_BUFFER->screen_y = 0;
  EDIT_BUFFER->offset_x = 0;
  EDIT_BUFFER->offset_y = 0;

  char* dsr_cruhex = bk_uint2hex(dsr->crubase);
  bk_strncpy(devpath, dsr_cruhex, 4);
  devpath[4] = '.';

  int err = 0;

  int existing = bk_existsFile(dsr, path);

  if (existing) {
    err = ed_loadFile(dsr, path);
  } else {
    ed_clearBuffer();
    EDIT_BUFFER->lineCount = 1;
  }

  if (!err) {
    int backup_nTitleline = nTitleLine;
    nTitleLine = 0;
    char cls[] = { 27, '[', '2', 'J', 0 };
    tputs_rom(cls);
    ed_renderLines(0, 0);

    conio_x = 0;
    conio_y = 0;

    ed_edit_loop(devpath);
    nTitleLine = backup_nTitleline;

    conio_x = 0;
    conio_y = displayHeight - 1;
  }

  if (sams_total_pages) {
    bk_free_pages(6);
  }
}

