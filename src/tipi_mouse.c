#include "banks.h"
#define MYBANK BANK(7)

#include "tipi_mouse.h"
#include "tipi_msg.h"
#include <vdp.h>
#include <kscan.h>
#include "globals.h"
#include "detect_vdp.h"
#include "terminal.h"

#define SUPPORT_MOUSE (vdp_type == VDP_F18A && displayHeight == 30)

extern char gfx_point0[32];
extern char gfx_point1[32];

void mouse_read(struct MouseData* mouseData) {
  char mousecode = 0x20;
  unsigned int readcount = 0;
  if (tipi_on()) {
    tipi_sendmsg(1, &mousecode);
    // contract with TIPI is that this will always read 3 bytes.
    tipi_recvmsg(&readcount, (char*)mouseData);
    tipi_off();
  }
}

void mouse_show(struct MouseData* mouseData) {
  if (SUPPORT_MOUSE) {
    mouseData->pointerx = 256/2;
    mouseData->pointery = 192/2;

    vdpmemcpy(gSpritePat + (8*0x60), gfx_point0, 32);
    vdpmemcpy(gSpritePat + 32 + (8*0x60), gfx_point1, 32);

    sprite(0, 0x60, COLOR_BLACK, mouseData->pointery, mouseData->pointerx);
    sprite(1, 0x64, COLOR_WHITE, mouseData->pointery, mouseData->pointerx);
    VDP_SET_REGISTER(0x33, 2);
  }
}

void mouse_hide() {
  if (SUPPORT_MOUSE) {
    VDP_SET_REGISTER(0x33, 0);
  }
}

void mouse_move(struct MouseData* mouseData) {
  if (SUPPORT_MOUSE) {
    mouse_read(mouseData);
    mouseData->pointerx += mouseData->mx;
    mouseData->pointery += mouseData->my;
    if (mouseData->pointerx < 8) {
      mouseData->pointerx = 8;
    } else if (mouseData->pointerx > 247) {
      mouseData->pointerx = 247;
    }
    if (mouseData->pointery < 0) {
      mouseData->pointery = 0;
    } else if (mouseData->pointery > 238) {
      mouseData->pointery = 238;
    }
    sprite_loc(0, mouseData->pointery, mouseData->pointerx);
    sprite_loc(1, mouseData->pointery, mouseData->pointerx);
  }
}
