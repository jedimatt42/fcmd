#ifndef _tipi_mouse
#define _tipi_mouse 1

#include "banking.h"

struct MouseData {
  char mx;
  char my;
  char buttons;
  int pointerx;
  int pointery;
};

#define MB_LEFT 0x01
#define MB_RIGHT 0x02
#define MB_MID 0x04

#define MP_NORMAL       0
#define MP_SCROLL_DOWN  2
#define MP_SCROLL_UP    3
#define MP_PAGE_DOWN    4
#define MP_PAGE_UP      5
#define MP_BUSY         6

void mouse_read(struct MouseData* mouseData);
void mouse_move(struct MouseData* mouseData);
void mouse_show(struct MouseData* mouseData);
void mouse_hide();
void mouse_set_pointer(int p);

DECLARE_BANKED_VOID(mouse_read, BANK(7), bk_tipi_mouse, (struct MouseData* mouseData), (mouseData))
DECLARE_BANKED_VOID(mouse_move, BANK(7), bk_updatePointer, (struct MouseData* mouseData), (mouseData))
DECLARE_BANKED_VOID(mouse_show, BANK(7), bk_enablePointer, (struct MouseData* mouseData), (mouseData))
DECLARE_BANKED_VOID(mouse_hide, BANK(7), bk_disablePointer, (), ())
DECLARE_BANKED_VOID(mouse_set_pointer, BANK(7), bk_setPointer, (int p), (p))

#endif

