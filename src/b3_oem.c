#include "banks.h"
#define MYBANK BANK(3)

#include "b3_oem.h"
#include <vdp.h>

void rom2vdp(int pAddr, char* pSrc, int cnt);

void defineChars() {
  charsetlc();
  rom2vdp(gPattern, &PAT0, 32 * 8);
  rom2vdp(gPattern + (127 * 8), &PAT127, 129 * 8);
  rom2vdp(gPattern + (0x5F * 8), &PAT95, 8);
}

void rom2vdp(int pAddr, char* pSrc, int cnt) {
  VDP_SET_ADDRESS_WRITE(pAddr);
  while (cnt--) {
    VDPWD=*(pSrc++);
  }
}

