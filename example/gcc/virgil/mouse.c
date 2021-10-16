#include <fc_api.h>
#include "mouse.h"
#include "page.h"
#include "gemini.h"

int mouseOn;
struct MouseData md;

void init_mouse() {
  mouseOn = 0;
  fc_strset((char*)&md, 0, sizeof(struct MouseData));
  fc_tipi_mouse_enable(&md); // prep for reading
}

int update_mouse() {
  md.buttons = 0;
  fc_tipi_mouse_move(&md);
  // if mouse is off, and we detect motion
  if (!mouseOn && (md.mx != 0 || md.my != 0)) {
    mouseOn = 1;
    fc_tipi_mouse_enable(&md);
  }
  return md.buttons;
}

static int mouse_line() {
  return (md.pointery / 8) + 1;
}

static int mouse_column() {
  return ((md.pointerx - 8) / 3) + 1;
}

int handle_mouse_click() {
  int line = mouse_line();
  if (line == 1) {
    int col = mouse_column();
    if (col > 74) { // quit button
      return 1;
    }
  }

  // screen line is 1 based, so decrement for that, and page is draw
  // one line down, so decrement for that.
  struct Line* page_line = page_get_line(line - 2 + state.line_offset);
  if (page_line->type == LINE_TYPE_LINK) {
    fc_ui_gotoxy(16,30);
    fc_tputs(page_line->data);
  }
  
  return 0;
}

