#include <fc_api.h>
#include <ioports.h>
#include "screen.h"
#include "page.h"
#include "gemini.h"
#include "version.h"
#include "link.h"
#include "mouse.h"

#define CBLACK_ON_GREEN (COLOR_BLACK << 4 | COLOR_MEDGREEN)
#define BLACK_ON_GRAY (COLOR_BLACK << 4 | COLOR_GRAY)
#define GREEN_ON_BLACK (COLOR_LTGREEN << 4 | COLOR_BLACK)
#define GRAY_ON_BLACK (COLOR_GRAY << 4 | COLOR_BLACK)
#define CYAN_ON_BLACK (COLOR_CYAN << 4 | COLOR_BLACK)
#define YELLOW_ON_BLACK (COLOR_LTYELLOW << 4 | COLOR_BLACK)
#define BROWN_ON_BLACK (COLOR_DKYELLOW << 4 | COLOR_BLACK)

struct DisplayInformation dinfo;
struct SamsInformation sams_info;

#define XYOFF(x, y) ((x-1) + ((y-1)*80))

// Screen Coordinates are 1 based like ANSI

int vdp_strcpy(int vdpaddr, char* str, int limit);

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
  vdp_memset(dinfo.colorAddr + XYOFF(1,30), CBLACK_ON_GREEN, 80);
  int addr = dinfo.imageAddr + XYOFF(1,30);
  vdp_memset(addr, ' ', 80);
  if (state.error[0] != 0) {
    vdp_strcpy(addr, state.error, 80);
    return;
  }
  if (state.loading) {
    addr += vdp_strcpy(addr, "Loading ", 8);
  } else {
    addr += vdp_strcpy(addr, "Line: ", 6);
    int offset = state.line_offset + 1;
    char* intstr = fc_uint2str(offset);
    addr += vdp_strcpy(addr, intstr, 5);
    addr += vdp_strcpy(addr, "-", 1);
    offset += 27;
    if (offset > state.line_count) { offset = state.line_count; }
    intstr = fc_uint2str(offset);
    addr += vdp_strcpy(addr, intstr, 5);
    addr += vdp_strcpy(addr, " of ", 4);
  }
  char* intstr = fc_uint2str(state.line_count);
  vdp_strcpy(addr, intstr, 5);
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
      if (line->length < 80) {
        vdp_memset(dinfo.imageAddr + line_offset + line->length, ' ', 80 - line->length);
      }
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
  fc_tipi_mouse_disable();
  fc_ui_gotoxy(1, 2);
  vdp_memset(dinfo.imageAddr + 80, ' ', 80 * 3);
  vdp_memset(dinfo.colorAddr + 80, CBLACK_ON_GREEN, 80 * 3);
  vdp_strcpy(dinfo.imageAddr + 80, prompt, 80);
  fc_ui_gotoxy(1,3);
  fc_bgcolor(COLOR_MEDGREEN);
  fc_textcolor(COLOR_BLACK);
  fc_getstr(dst, 79, 1);
  screen_redraw();
  fc_tipi_mouse_enable(&md);
}

int vdp_strcpy(int vdpaddr, char* str, int limit) {
  VDP_SET_ADDRESS_WRITE(vdpaddr);
  int i = 0;
  while((str[i] != 0) && (i < limit)) {
    VDPWD = str[i++];
  }
  return i;
}
