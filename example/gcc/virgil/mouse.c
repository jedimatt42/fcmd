#include <fc_api.h>
#include "mouse.h"

int mouseOn;
struct MouseData md;

void init_mouse() {
  mouseOn = 0;
  fc_strset((char*)&md, 0, sizeof(struct MouseData));
  fc_tipi_mouse_enable(&md); // prep for reading
  fc_tipi_mouse_disable(); // hide the pointer
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

  return 0;
}

