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

extern char gfx_ptr_normal_black[32];
extern char gfx_ptr_normal_white[32];
extern char gfx_ptr_busy_black[32];
extern char gfx_ptr_busy_white[32];
extern char gfx_ptr_scroll_up_black[32];
extern char gfx_ptr_scroll_up_white[32];
extern char gfx_ptr_scroll_down_black[32];
extern char gfx_ptr_scroll_down_white[32];
extern char gfx_ptr_page_up_black[32];
extern char gfx_ptr_page_up_white[32];
extern char gfx_ptr_page_down_black[32];
extern char gfx_ptr_page_down_white[32];

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

void mouse_set_pointer(int p) {
  if (!SUPPORT_MOUSE) return;
  const char* black;
  const char* white;
  switch (p) {
    case MP_BUSY:
      black = gfx_ptr_busy_black;
      white = gfx_ptr_busy_white;
      break;
    case MP_SCROLL_UP:
      black = gfx_ptr_scroll_up_black;
      white = gfx_ptr_scroll_up_white;
      break;
    case MP_SCROLL_DOWN:
      black = gfx_ptr_scroll_down_black;
      white = gfx_ptr_scroll_down_white;
      break;
    case MP_PAGE_UP:
      black = gfx_ptr_page_up_black;
      white = gfx_ptr_page_up_white;
      break;
    case MP_PAGE_DOWN:
      black = gfx_ptr_page_down_black;
      white = gfx_ptr_page_down_white;
      break;
    default:
      black = gfx_ptr_normal_black;
      white = gfx_ptr_normal_white;
      break;
  }
  vdpmemcpy(gSpritePat + (8*0x60), black, 32);
  vdpmemcpy(gSpritePat + 32 + (8*0x60), white, 32);
}

void mouse_show(struct MouseData* mouseData) {
  if (SUPPORT_MOUSE) {
    mouseData->pointerx = 256/2;
    mouseData->pointery = 192/2;

    mouse_set_pointer(MP_NORMAL);

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
