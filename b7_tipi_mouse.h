#ifndef _tipi_mouse
#define _tipi_mouse 1

#include "banking.h"

struct __attribute__((__packed__)) MouseData {
  char mx;
  char my;
  char buttons;
  int pointerx;
  int pointery;
};

#define MB_LEFT 0x01
#define MB_RIGHT 0x02
#define MB_MID 0x04

void tipi_mouse(struct MouseData* mouseData);
void updatePointer(struct MouseData* mouseData);
void enablePointer(struct MouseData* mouseData);
void disablePointer();

DECLARE_BANKED_VOID(tipi_mouse, BANK(7), bk_tipi_mouse, (struct MouseData* mouseData), (mouseData))
DECLARE_BANKED_VOID(updatePointer, BANK(7), bk_updatePointer, (struct MouseData* mouseData), (mouseData))
DECLARE_BANKED_VOID(enablePointer, BANK(7), bk_enablePointer, (struct MouseData* mouseData), (mouseData))
DECLARE_BANKED_VOID(disablePointer, BANK(7), bk_disablePointer, (), ())

#endif

