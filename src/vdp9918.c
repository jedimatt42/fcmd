#include "banks.h"
#define MYBANK BANK(3)

#include "vdp9918.h"
#include <conio.h>
#include <vdp.h>

void vdp9918_init_text40() {
  VDP_SET_REGISTER(0, 0x00);
  VDP_SET_REGISTER(1, gUnblank);
  VDP_SET_REGISTER(2, 0x00);
  VDP_SET_REGISTER(3, 0x0E);
  VDP_SET_REGISTER(4, 0x01);
  VDP_SET_REGISTER(5, 0x06);
  VDP_SET_REGISTER(6, 0x00);
  VDP_SET_REGISTER(7, conio_scrnCol);
}
