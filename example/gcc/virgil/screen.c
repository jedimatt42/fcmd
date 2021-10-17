#include <fc_api.h>
#include <ioports.h>
#include "screen.h"
#include "page.h"
#include "gemini.h"
#include "version.h"
#include "link.h"

#define COLOR_TRANS 0x00
#define COLOR_BLACK 0x01
#define COLOR_MEDGREEN 0x02
#define COLOR_LTGREEN 0x03
#define COLOR_DKBLUE 0x04
#define COLOR_LTBLUE 0x05
#define COLOR_DKRED 0x06
#define COLOR_CYAN 0x07
#define COLOR_MEDRED 0x08
#define COLOR_LTRED 0x09
#define COLOR_DKYELLOW 0x0A
#define COLOR_LTYELLOW 0x0B
#define COLOR_DKGREEN 0x0C
#define COLOR_MAGENTA 0x0D
#define COLOR_GRAY 0x0E
#define COLOR_WHITE 0x0F

const char BLACK_ON_GREEN[9] = "\033[30;42m";
const unsigned char CBLACK_ON_GREEN = (COLOR_BLACK << 4 | COLOR_MEDGREEN);
const unsigned char BLACK_ON_GRAY = (COLOR_BLACK << 4 | COLOR_GRAY);
const unsigned char GREEN_ON_BLACK = (COLOR_LTGREEN << 4 | COLOR_BLACK);
const unsigned char GRAY_ON_BLACK = (COLOR_GRAY << 4 | COLOR_BLACK);
const unsigned char CYAN_ON_BLACK = (COLOR_CYAN << 4 | COLOR_BLACK);

struct DisplayInformation dinfo;

// Screen Coordinates are 1 based like ANSI

void init_screen() {
  fc_display_info(&dinfo);
  // use CLS after setting color to force border and all 
  // attributes.
  fc_exec("COLOR 14 1");
  fc_exec("CLS");
  screen_status();
}

void screen_title() {
  fc_ui_gotoxy(1, 1);
  fc_tputs(BLACK_ON_GREEN);
  vdp_memset(dinfo.colorAddr, CBLACK_ON_GREEN, 80);
  fc_tputs("-< " VERSION " >-");
  fc_ui_gotoxy(69, 1);
  if (state.loading) {
    fc_tputs("[STOP]");
  } else {
    fc_tputs("[BACK]");
  }
  fc_tputs("[QUIT]");
  vdp_memset(dinfo.colorAddr, CBLACK_ON_GREEN, 80);
} 

void screen_status() {
  screen_title();
  vdp_memset(dinfo.imageAddr + (29 * 80), ' ', 80);
  vdp_memset(dinfo.colorAddr + (29 * 80), CBLACK_ON_GREEN, 80);
  fc_ui_gotoxy(1, 30);
  fc_tputs(BLACK_ON_GREEN);
  if (state.error[0] != 0) {
    fc_tputs(state.error);
    fc_tputs(" ");
    fc_tputs(state.url);
    return;
  }
  if (state.loading) {
    fc_tputs("Loading ");
  } else {
    fc_tputs("Line: ");
    fc_tputs(fc_uint2str(state.line_offset + 1));
    fc_tputs(" of ");
  }
  fc_tputs(fc_uint2str(state.line_count));
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
    struct Line* line = page_get_line(i + state.line_offset);
    int line_offset = (i+1) * 80;
    if (line->type == LINE_TYPE_LINK) {
      int len = 0;
      char* label = link_label(line->data, &len);
      vdp_memcpy(dinfo.imageAddr + line_offset, label, len);
      vdp_memset(dinfo.imageAddr + line_offset + len, ' ', 80 - len);
      vdp_memset(dinfo.colorAddr + line_offset, GREEN_ON_BLACK, 80);
    } else {
      unsigned char color = GRAY_ON_BLACK;
      if (line->type == LINE_TYPE_HEADING) {
	color = CYAN_ON_BLACK;
      } else if (line->type == LINE_TYPE_LITERAL) {
	color = GRAY_ON_BLACK;
      }
      int line_offset = (i+1) * 80;
      vdp_memcpy(dinfo.imageAddr + line_offset, line->data, line->length);
      vdp_memset(dinfo.imageAddr + line_offset + line->length, ' ', 80 - line->length);
      vdp_memset(dinfo.colorAddr + line_offset, color, 80);
    }
  }
  // blank remaining lines.
  for(int i = state.line_count; i < 28; i++) {
    vdp_memset(dinfo.imageAddr + ((i+1) * 80), ' ', 80);
    vdp_memset(dinfo.colorAddr + ((i+1) * 80), GRAY_ON_BLACK, 80);
  }
  screen_status();
}

