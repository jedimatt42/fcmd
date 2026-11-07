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

int mouse_active;

void on_menu();

void FC_SAMS(0,init_mouse()) {
  str_set((char*)&md, 0, sizeof(struct MouseData));
  mouse_show(&md); // prep for reading
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
  mouse_move(&md);
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
	    str_copy(state.newurl, "bookmarks:");
	  } else if (line == 4) {
	    state.cmd = CMD_IDLE;
	    bookmarks_add_link(state.lasturl);
	  } else if (line == 5) {
	    str_copy(state.newurl, "history:");
	  } else if (line == 6) {
	    str_copy(state.newurl, "about:");
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


