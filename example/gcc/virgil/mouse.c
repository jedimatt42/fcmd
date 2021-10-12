#include <fc_api.h>
#include "mouse.h"

int mouseOn;
struct MouseData md;
struct MouseData* mouseData;

void init_mouse() {
  mouseOn = 0;
  fc_strset((char*)&md, 0, sizeof(struct MouseData));
  mouseData = &md;
  fc_tipi_mouse_enable(&md); // prep for reading
  fc_tipi_mouse_disable(); // hide the pointer
}

int update_mouse() {
  mouseData->buttons = 0;
  fc_tipi_mouse_move(&md);
  if (!mouseOn && (md.mx != 0 || md.my != 0)) {
    mouseOn = 1;
    fc_tipi_mouse_enable(&md);
  }
  return md.buttons;
}


