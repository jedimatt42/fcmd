#include <fc_api.h>
#include <ioports.h>
#include "mouse.h"
#include "page.h"
#include "gemini.h"
#include "link.h"
#include "screen.h"
#include "keyboard.h"
#include "history.h"
#include "bookmarks.h"

struct MouseData md;
int pointer_type;

int mouse_active;

static int spritePatternAddr;

void on_menu();

void FC_SAMS(0,init_mouse()) {
  struct DisplayInformation dinfo;
  fc_display_info(&dinfo);
  spritePatternAddr = dinfo.spritePatternAddr;

  fc_strset((char*)&md, 0, sizeof(struct MouseData));
  fc_tipi_mouse_enable(&md); // prep for reading
  pointer_type = 99;
  mouse_set_pointer(MP_NORMAL);
  mouse_active = 0;
}

static int mouse_line() {
  return (md.pointery / 8) + 1;
}

static int mouse_column() {
  return ((md.pointerx - 8) / 3) + 1;
}

int FC_SAMS(0,update_mouse()) {
  md.buttons = 0;
  int oldy = md.pointery;
  fc_tipi_mouse_move(&md);
  if (oldy != md.pointery) {
    // mouse moved, so let's set the active flag
    // this can be a hint to other stages to allow
    // more time for ui.
    mouse_active = 3;
  }

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
    mouse_set_pointer(state.cmd == CMD_IDLE ? MP_NORMAL : MP_BUSY);
  }
  return md.buttons;
}

void FC_SAMS(0,handle_mouse_click()) {
  while(MB_LEFT & update_mouse()) {
    // only handle click after mouse-button-up
  }
  int line = mouse_line();
  int col = mouse_column();
  if (line == 1) {
    if (col >= XQUIT && col < (XQUIT + 6)) { // quit button
      state.cmd = CMD_QUIT;
      return;
    } else if (col >= XSTOP && col < XSTOP + 6) {
      if (state.cmd == CMD_READPAGE) {
	// stop button
	state.cmd = CMD_STOP;
      } else {
	// back button
	on_back();
      }
      return;
    } else if (col >= XMENU && col < XMENU + 5) {
      if (!state.menu_open) {
        on_menu();
        return;
      }
    }
  } else {
    if (state.menu_open) {  
      if (col >= XMENU && col <= XMENU + 11 && line <= 6) {
	screen_redraw();
	if (line == 2) {
	  on_address();
	} else {
          state.cmd = CMD_RELOAD;
	  if (line == 3) {
	    fc_strcpy(state.newurl, "bookmarks:");
	  } else if (line == 4) {
	    state.cmd = CMD_IDLE;
	    bookmarks_add_link(state.lasturl);
	  } else if (line == 5) {
	    fc_strcpy(state.newurl, "history:");
	  } else if (line == 6) {
	    fc_strcpy(state.newurl, "about:");
	  }
	}
	state.menu_open = 0;
	return;
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
	return;
      }
    }
  } 

  // page is draw one line down, so decrement for that.
  int line_id = line - 1 + state.line_offset;
  struct Line* page_line = page_get_line(line_id);
  if (page_line->type == LINE_TYPE_LINK || page_line->type == LINE_TYPE_LINK_CONT) {
    link_set_url(state.newurl, line_id);
    state.cmd = CMD_RELOAD;
  } else if (state.menu_open) {
    state.menu_open = 0;
    screen_redraw();
  }
  
  return;
}

void on_menu() {
  state.cmd = CMD_IDLE;
  screen_menu();
  state.menu_open = 1;
}

#define SPR16X(a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p) { \
  a >> 8, b >> 8, c >> 8, d >> 8, e >> 8, f >> 8, g >> 8, h >> 8, \
  i >> 8, j >> 8, k >> 8, l >> 8, m >> 8, n >> 8, o >> 8, p >> 8, \
  a & 0xff, b & 0xff, c & 0xff, d & 0xff, e & 0xff, f & 0xff, g & 0xff, h & 0xff, \
  i & 0xff, j & 0xff, k & 0xff, l & 0xff, m & 0xff, n & 0xff, o & 0xff, p & 0xff \
}

const char scroll_down_pointer_white[32] = SPR16X(
    0b0000000000000000,
    0b0100000100000000,
    0b0110001100000000,
    0b0011011000000000,
    0b0001110000000000,
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

const char scroll_down_pointer_black[32] = SPR16X(
    0b0100000100000000,
    0b1010001010000000,
    0b1001010010000000,
    0b0100100100000000,
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
    0b0000000000000000
);

const char scroll_up_pointer_white[32] = SPR16X(
    0b0000000000000000,
    0b0000100000000000,
    0b0001110000000000,
    0b0011011000000000,
    0b0110001100000000,
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

const char scroll_up_pointer_black[32] = SPR16X(
    0b0000100000000000,
    0b0001010000000000,
    0b0010001000000000,
    0b0100100100000000,
    0b1001010010000000,
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
    0b0000000000000000
);

const char page_down_pointer_white[32] = SPR16X(
    0b0000000000000000,
    0b0100000100000000,
    0b0110001100000000,
    0b0011011000000000,
    0b0001110000000000,
    0b0000100000000000,
    0b0000000000000000,
    0b0100000100000000,
    0b0110001100000000,
    0b0011011000000000,
    0b0001110000000000,
    0b0000100000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

const char page_down_pointer_black[32] = SPR16X(
    0b0100000100000000,
    0b1010001010000000,
    0b1001010010000000,
    0b0100100100000000,
    0b0010001000000000,
    0b0001010000000000,
    0b0100100100000000,
    0b1010001010000000,
    0b1001010010000000,
    0b0100100100000000,
    0b0010001000000000,
    0b0001010000000000,
    0b0000100000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

const char page_up_pointer_white[32] = SPR16X(
    0b0000000000000000,
    0b0000100000000000,
    0b0001110000000000,
    0b0011011000000000,
    0b0110001100000000,
    0b0100000100000000,
    0b0000000000000000,
    0b0000100000000000,
    0b0001110000000000,
    0b0011011000000000,
    0b0110001100000000,
    0b0100000100000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

const char page_up_pointer_black[32] = SPR16X(
    0b0000100000000000,
    0b0001010000000000,
    0b0010001000000000,
    0b0100100100000000,
    0b1001010010000000,
    0b1010001010000000,
    0b0100100100000000,
    0b0001010000000000,
    0b0010001000000000,
    0b0100100100000000,
    0b1001010010000000,
    0b1010001010000000,
    0b0100000100000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
);

const char normal_pointer_white[32] = SPR16X(
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

const char normal_pointer_black[32] = SPR16X(
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

const char busy_pointer_white[32] = SPR16X(
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
    0b0000000011110000,
    0b0011110000100000,
    0b0000100001000000,
    0b0001000011110000,
    0b0011110000000000,
    0b0000000000000000
);

const char busy_pointer_black[32] = SPR16X(
    0b1111111100000000,
    0b1000000100000000,
    0b1000001000000000,
    0b1000010000000000,
    0b1000001000000000,
    0b1001000100000000,
    0b1010100010000000,
    0b1100010001000000,
    0b0000001010000000,
    0b0000000111111000,
    0b0111111100001000,
    0b0100001111011000,
    0b0111011110111000,
    0b0110111100001000,
    0b0100001111111000,
    0b0111111000000000
);

void FC_SAMS(0,mouse_set_pointer(int p)) {
  if (pointer_type != p) {
    pointer_type = p;
    const char* black;
    const char* white;
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
    } else if (pointer_type == MP_BUSY) {
      black = busy_pointer_black;
      white = busy_pointer_white;
    } else {
      black = normal_pointer_black;
      white = normal_pointer_white;
    }
    vdp_memcpy(spritePatternAddr + (8*0x60), black, 32);
    vdp_memcpy(spritePatternAddr + 32 + (8*0x60), white, 32);
  }
}


