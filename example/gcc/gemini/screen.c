#include <fc_api.h>
#include <ioports.h>
#include "screen.h"
#include "page.h"
#include "gemini.h"

const char BLACK_ON_GREEN[9] = "\033[30;42m";
const char GREEN_ON_BLACK[9] = "\033[32;40m";
const char GRAY_ON_BLACK[9] = "\033[90;40m";
const char CYAN_ON_BLACK[9] = "\033[96;40m";

struct DisplayInformation dinfo;

// Screen Coordinates are 1 based like ANSI

void init_screen() {
  fc_display_info(&dinfo);
  // use CLS after setting color to force border and all 
  // attributes.
  fc_exec("COLOR 14 1");
  fc_exec("CLS");
  fc_ui_gotoxy(1, 1);
  fc_tputs(BLACK_ON_GREEN);
  fc_tputs("-< GEMINI TI-99/4A Browser v1.0 >-");
  fc_ui_gotoxy(1, 30);
  fc_tputs("Line: 0 of 0");
} 

void screen_scroll_to(int lineno) {
  if (lineno < state.line_count) {
    state.line_offset = lineno;
  }
}

void screen_redraw() {
  int limit = state.line_count - state.line_offset;
  limit = limit < 28 ? limit : 28;
  for(int i = 0; i < limit; i++) {
    fc_ui_gotoxy(1, i + 2);
    struct Line* line = page_get_line(i + state.line_offset);
    if (line->data[0] == '#') {
      fc_tputs(CYAN_ON_BLACK);
    } else if (line->data[0] == '=' && line->data[1] == '>') {
      fc_tputs(GREEN_ON_BLACK);
    } else {
      fc_tputs(GRAY_ON_BLACK);
    }
    fc_tputs(page_get_line(i)->data);
  }
  // blank remaining lines.
  for(int i = state.line_count; i < 28; i++) {
    vdp_memset((i+1) * 80, ' ', 80);
  }
}

