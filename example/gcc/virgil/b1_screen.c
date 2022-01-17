#include <fc_api.h>
#include <ioports.h>
#include "screen.h"
#include "page.h"
#include "gemini.h"
#include "version.h"
#include "link.h"
#include "mouse.h"
#include "debug.h"

#define CBLACK_ON_GREEN (COLOR_BLACK << 4 | COLOR_MEDGREEN)
#define BLACK_ON_GRAY (COLOR_BLACK << 4 | COLOR_GRAY)
#define GREEN_ON_BLACK (COLOR_LTGREEN << 4 | COLOR_BLACK)
#define GRAY_ON_BLACK (COLOR_GRAY << 4 | COLOR_BLACK)
#define WHITE_ON_BLACK (COLOR_WHITE << 4 | COLOR_BLACK)
#define CYAN_ON_BLACK (COLOR_CYAN << 4 | COLOR_BLACK)
#define YELLOW_ON_BLACK (COLOR_LTYELLOW << 4 | COLOR_BLACK)
#define BROWN_ON_BLACK (COLOR_DKYELLOW << 4 | COLOR_BLACK)

#define XYOFF(x, y) ((x-1) + ((y-1)*80))

// Screen Coordinates are 1 based like ANSI

int imageAddr;
int colorAddr;

void FC_SAMS(1,init_screen()) {
  // use CLS after setting color to force border and all 
  // attributes.
  fc_exec("COLOR 14 1");
  fc_exec("CLS");

  struct DisplayInformation dinfo;
  fc_display_info(&dinfo);
  imageAddr = dinfo.imageAddr;
  colorAddr = dinfo.colorAddr;
	  
  screen_status();
}

// these are forced to functions to make stepping through the debugger easier
//  not for any other technical reason

int __attribute__((noinline)) write_string(int offset, char* str, int limit) {
  VDP_SET_ADDRESS_WRITE(imageAddr + offset);
  int i = 0;
  while((str[i] != 0) && (i < limit)) {
    VDPWD = str[i++];
  }
  return i;
}

void __attribute__((noinline)) set_line_color(int offset, int color) {
  vdp_memset(colorAddr + offset, color, 80);
}

void __attribute__((noinline)) set_line_text(int offset, char* text) {
  vdp_memcpy(imageAddr + offset, text, 80);
}

void FC_SAMS(1,screen_title()) {
  struct SamsInformation sams_info;
  fc_sams_info(&sams_info);

  fc_ui_gotoxy(1, 1);
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

  i = XMENU - 1;
  tmp[i++] = BL;
  tmp[i++] = 0x19; // down arrow
  fc_strcpy(tmp + i, "MENU");
  i += 4;
  tmp[i++] = BR;

  i++;
  tmp[i++] = BL;
  if (state.cmd == CMD_READPAGE) {
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

  set_line_text(0, tmp);

  set_line_color(0, CBLACK_ON_GREEN);
} 

void FC_SAMS(1,screen_status()) {
  screen_title();
  set_line_color(XYOFF(1,30), CBLACK_ON_GREEN);
  int offset = XYOFF(1,30);
  vdp_memset(offset, ' ', 80);
  if (state.error[0] != 0) {
    write_string(offset, state.error, 80);
    return;
  }
  if (state.cmd == CMD_READPAGE) {
    offset += write_string(offset, "Loading ", 8);
  } else {
    offset += write_string(offset, "Line: ", 6);
    int lineno = state.line_offset + 1;
    char* intstr = fc_uint2str(lineno);
    offset += write_string(offset, intstr, 5);
    offset += write_string(offset, "-", 1);
    lineno += 27;
    if (lineno > state.line_count) { lineno = state.line_count; }
    intstr = fc_uint2str(lineno);
    offset += write_string(offset, intstr, 5);
    offset += write_string(offset, " of ", 4);
  }
  char* intstr = fc_uint2str(state.line_count);
  write_string(offset, intstr, 5);
}

void FC_SAMS(1,screen_scroll_to(int lineno)) {
  if (lineno < state.line_count) {
    state.line_offset = lineno;
  }
}

void FC_SAMS(1,screen_redraw()) {
  int limit = state.line_count - state.line_offset;
  limit = limit < 28 ? limit : 28;
  int i = 0;
  int vdp_line_offset = 80;
  unsigned char color = GRAY_ON_BLACK;
  while(i < limit) {
    struct Line* line = page_get_line(i + state.line_offset + 1);
    if (line->type == LINE_TYPE_LINK_CONT) {
      // use same color as previous link line
      vdp_memcpy(imageAddr + vdp_line_offset, line->data, 80);
      vdp_memset(colorAddr + vdp_line_offset, color, 80);
    } else if (line->type == LINE_TYPE_LINK) {
      int len = 0;
      char* url = link_url_scheme(line->data, &len);
      color = GREEN_ON_BLACK;
      if (fc_str_startswith(url, "http")) {
	if (url[4] == 's' || (url[4] == 0)) {
	  color = YELLOW_ON_BLACK;
	}
      } else if (fc_str_startswith(url, "gopher:")) {
	color = BROWN_ON_BLACK;
      }
      char* label = link_label(line->data, &len);
      vdp_memcpy(imageAddr + vdp_line_offset, label, len);
      vdp_memset(imageAddr + vdp_line_offset + len, ' ', 80 - len);
      vdp_memset(colorAddr + vdp_line_offset, color, 80);
    } else {
      color = GRAY_ON_BLACK;
      if (line->type == LINE_TYPE_HEADING) {
	color = CYAN_ON_BLACK;
      } else if (line->type == LINE_TYPE_LITERAL) {
	color = WHITE_ON_BLACK;
      }
      vdp_memcpy(imageAddr + vdp_line_offset, line->data, 80);
      vdp_memset(colorAddr + vdp_line_offset, color, 80);
    }
    i++;
    vdp_line_offset += 80;
  }

  // blank remaining lines.
  for(int i = state.line_count; i < 28; i++) {
    vdp_memset(imageAddr + ((i+1) * 80), ' ', 80);
    vdp_memset(colorAddr + ((i+1) * 80), GRAY_ON_BLACK, 80);
  }
}

void FC_SAMS(1,screen_prompt(char* dst, char* prompt)) {
  fc_tipi_mouse_disable();
  fc_ui_gotoxy(1, 2);
  vdp_memset(imageAddr + 80, ' ', 80 * 3);
  vdp_memset(colorAddr + 80, CBLACK_ON_GREEN, 80 * 3);
  write_string(80, prompt, 80);
  fc_ui_gotoxy(1,3);
  fc_bgcolor(COLOR_MEDGREEN);
  fc_textcolor(COLOR_BLACK);
  fc_getstr(dst, 79, 1);
  screen_redraw();
  fc_tipi_mouse_enable(&md);
}

void FC_SAMS(1,screen_menu()) {
  for(int i=1; i < 4; i++) {
    vdp_memset(imageAddr + (i*80) + XMENU, ' ', 10);
    vdp_memset(colorAddr + (i*80) + XMENU, CBLACK_ON_GREEN, 10);
  }
  int mAddr = imageAddr + 80 + XMENU + 1;
  vdp_memcpy(mAddr, "Address", 7);
  vdp_memcpy(mAddr + 80, "History", 7);
  vdp_memcpy(mAddr + 160, "About", 5);
}
