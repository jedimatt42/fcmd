#include <fc_api.h>
#include <ioports.h>
#include "mouse.h"
#include "page.h"
#include "gemini.h"
#include "link.h"
#include "screen.h"
#include "keyboard.h"

struct MouseData md;
int pointer_type;

void init_mouse() {
  fc_strset((char*)&md, 0, sizeof(struct MouseData));
  fc_tipi_mouse_enable(&md); // prep for reading
  pointer_type = 99;
  mouse_set_pointer(MP_NORMAL);
}

static int mouse_line() {
  return (md.pointery / 8) + 1;
}

static int mouse_column() {
  return ((md.pointerx - 8) / 3) + 1;
}

int update_mouse() {
  md.buttons = 0;
  fc_tipi_mouse_move(&md);
  int line = mouse_line();
  if (line > 1 && mouse_column() > 70) {
    if (line < 8) {
      mouse_set_pointer(MP_PAGE_UP);
    } else if (line < 16) {
      mouse_set_pointer(MP_SCROLL_UP);
    } else if (line < 24) {
      mouse_set_pointer(MP_SCROLL_DOWN);
    } else {
      mouse_set_pointer(MP_PAGE_DOWN);
    }
  } else {
    mouse_set_pointer(MP_NORMAL);
  }
  return md.buttons;
}

int handle_mouse_click() {
  while(MB_LEFT & update_mouse()) {
    // only handle click after mouse-button-up
  }
  int line = mouse_line();
  int col = mouse_column();
  if (line == 1) {
    if (col > 74) { // quit button
      return 1;
    }
  } else {
    int redraw = 0;
    if (col > 70) {
      if (line < 8) {
	redraw = on_page_up();
      } else if (line < 16) {
	redraw = on_key_up();
      } else if (line < 24) {
	redraw = on_key_down();
      } else {
	redraw = on_page_down();
      }
      if (redraw) {
	screen_redraw();
      }
      return 0;
    }
  } 

  // screen line is 1 based, so decrement for that, and page is draw
  // one line down, so decrement for that.
  struct Line* page_line = page_get_line(line - 2 + state.line_offset);
  if (page_line->type == LINE_TYPE_LINK) {
    int junk;
    open_url(link_url(page_line->data, &junk));
  }
  
  return 0;
}

#define SPR16X(a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p) { \
  a >> 8, b >> 8, c >> 8, d >> 8, e >> 8, f >> 8, g >> 8, h >> 8, \
  i >> 8, j >> 8, k >> 8, l >> 8, m >> 8, n >> 8, o >> 8, p >> 8, \
  a & 0xff, b & 0xff, c & 0xff, d & 0xff, e & 0xff, f & 0xff, g & 0xff, h & 0xff, \
  i & 0xff, j & 0xff, k & 0xff, l & 0xff, m & 0xff, n & 0xff, o & 0xff, p & 0xff \
}

char scroll_down_pointer_white[32] = SPR16X(
    0b0000000000000000,
    0b0100000100000000,
    0b0010001000000000,
    0b0001010000000000,
    0b0000100000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

char scroll_down_pointer_black[32] = SPR16X(
    0b0100000100000000,
    0b1010001010000000,
    0b0101010100000000,
    0b0010101000000000,
    0b0001010000000000,
    0b0000100000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

char scroll_up_pointer_white[32] = SPR16X(
    0b0000000000000000,
    0b0000100000000000,
    0b0001010000000000,
    0b0010001000000000,
    0b0100000100000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

char scroll_up_pointer_black[32] = SPR16X(
    0b0000100000000000,
    0b0001010000000000,
    0b0010101000000000,
    0b0101010100000000,
    0b1010001010000000,
    0b0100000100000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

char page_down_pointer_white[32] = SPR16X(
    0b0000000000000000,
    0b0100000100000000,
    0b0010001000000000,
    0b0001010000000000,
    0b0000100000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0100000100000000,
    0b0010001000000000,
    0b0001010000000000,
    0b0000100000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

char page_down_pointer_black[32] = SPR16X(
    0b0100000100000000,
    0b1010001010000000,
    0b0101010100000000,
    0b0010101000000000,
    0b0001010000000000,
    0b0000100000000000,
    0b0100000100000000,
    0b1010001010000000,
    0b0101010100000000,
    0b0010101000000000,
    0b0001010000000000,
    0b0000100000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

char page_up_pointer_white[32] = SPR16X(
    0b0000000000000000,
    0b0000100000000000,
    0b0001010000000000,
    0b0010001000000000,
    0b0100000100000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000100000000000,
    0b0001010000000000,
    0b0010001000000000,
    0b0100000100000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

char page_up_pointer_black[32] = SPR16X(
    0b0000100000000000,
    0b0001010000000000,
    0b0010101000000000,
    0b0101010100000000,
    0b1010001010000000,
    0b0100000100000000,
    0b0000100000000000,
    0b0001010000000000,
    0b0010101000000000,
    0b0101010100000000,
    0b1010001010000000,
    0b0100000100000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

char normal_pointer_white[32] = SPR16X(
    0b0000000000000000,
    0b0111111000000000,
    0b0111110000000000,
    0b0111100000000000,
    0b0111110000000000,
    0b0110111000000000,
    0b0100011100000000,
    0b0000001110000000,
    0b0000000100000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

char normal_pointer_black[32] = SPR16X(
    0b1111111100000000,
    0b1000000100000000,
    0b1000001000000000,
    0b1000010000000000,
    0b1000001000000000,
    0b1001000100000000,
    0b1010100010000000,
    0b1100010001000000,
    0b0000001010000000,
    0b0000000100000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

void mouse_set_pointer(int p) {
  if (pointer_type != p) {
    pointer_type = p;
    char* black = normal_pointer_black;
    char* white = normal_pointer_white;
    if (pointer_type == MP_PAGE_UP) {
      black = page_up_pointer_black;
      white = page_up_pointer_white;
    } else if (pointer_type == MP_PAGE_DOWN) {
      black = page_down_pointer_black;
      white = page_down_pointer_white;
    } else if (pointer_type == MP_SCROLL_UP) {
      black = scroll_up_pointer_black;
      white = scroll_up_pointer_white;
    } else if (pointer_type == MP_SCROLL_DOWN) {
      black = scroll_down_pointer_black;
      white = scroll_down_pointer_white;
    } else {
      black = normal_pointer_black;
      white = normal_pointer_white;
    }
    vdp_memcpy(dinfo.spritePatternAddr + (8*0x60), black, 32);
    vdp_memcpy(dinfo.spritePatternAddr + 32 + (8*0x60), white, 32);
  }
}


