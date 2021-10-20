#include <fc_api.h>
#include <ioports.h>
#include "screen.h"
#include "page.h"
#include "gemini.h"
#include "version.h"
#include "link.h"

#define BLACK_ON_GREEN "\033[30;42m"
#define CBLACK_ON_GREEN (COLOR_BLACK << 4 | COLOR_MEDGREEN)
#define BLACK_ON_GRAY (COLOR_BLACK << 4 | COLOR_GRAY)
#define GREEN_ON_BLACK (COLOR_LTGREEN << 4 | COLOR_BLACK)
#define GRAY_ON_BLACK (COLOR_GRAY << 4 | COLOR_BLACK)
#define CYAN_ON_BLACK (COLOR_CYAN << 4 | COLOR_BLACK)
#define YELLOW_ON_BLACK (COLOR_LTYELLOW << 4 | COLOR_BLACK)
#define BROWN_ON_BLACK (COLOR_DKYELLOW << 4 | COLOR_BLACK)

struct DisplayInformation dinfo;
struct SamsInformation sams_info;

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
  fc_sams_info(&sams_info);
  fc_ui_gotoxy(1, 1);
  vdp_memset(dinfo.colorAddr, CBLACK_ON_GREEN, 80);
  char tmp[80];
  fc_strset(tmp, BS, 80);
  tmp[1] = BL;
  tmp[2] = ' ';
  fc_strcpy(tmp + 3, VERSION);
  int i = fc_strlen(tmp);
  tmp[i++] = ' ';
  tmp[i++] = BR;
  i+=2;
  tmp[i++] = BL;
  fc_strcpy(tmp + i, fc_uint2str(sams_info.next_page * 4));
  i += fc_strlen(tmp + i);
  tmp[i++] = 'K';
  tmp[i++] = '/';
  fc_strcpy(tmp + i, fc_uint2str(sams_info.total_pages * 4));
  i += fc_strlen(tmp + i);
  tmp[i++] = 'K';
  tmp[i++] = BR;

  i = XADDRESS - 1;
  tmp[i++] = BL;
  fc_strcpy(tmp + i, "ADDRESS");
  i += 7;
  tmp[i++] = BR;

  i++;
  tmp[i++] = BL;
  if (state.loading) {
    fc_strcpy(tmp + i, "STOP");
  } else {
    fc_strcpy(tmp + i, "BACK");
  }
  i += 4;
  tmp[i++] = BR;
  i++;

  tmp[i++] = BL;
  fc_strcpy(tmp + i, "QUIT");
  i += 4;
  tmp[i] = BR;

  vdp_memcpy(dinfo.imageAddr, tmp, 80);
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
      char* url = link_url(line->data, &len);
      int color = GREEN_ON_BLACK;
      if (fc_str_startswith(url, "http")) {
	if (url[4] == ':' || (url[4] == 's' && url[5] == ':')) {
	  color = YELLOW_ON_BLACK;
	}
      } else if (fc_str_startswith(url, "gopher:")) {
	color = BROWN_ON_BLACK;
      }
      char* label = link_label(line->data, &len);
      vdp_memcpy(dinfo.imageAddr + line_offset, label, len);
      vdp_memset(dinfo.imageAddr + line_offset + len, ' ', 80 - len);
      vdp_memset(dinfo.colorAddr + line_offset, color, 80);
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

void screen_prompt(char* dst, char* prompt) {
  fc_ui_gotoxy(1, 2);
  fc_tputs(BLACK_ON_GREEN);
  vdp_memset(dinfo.imageAddr + 80, ' ', 80 * 3);
  vdp_memset(dinfo.colorAddr + 80, CBLACK_ON_GREEN, 80 * 3);
  fc_tputs(prompt);
  fc_ui_gotoxy(1,3);
  fc_getstr(dst, 79, 1);
  screen_redraw();
}


