#ifndef _DETECT_VDP_H
#define _DETECT_VDP_H 1

#include "banking.h"

extern int vdp_type;

#define VDP_F18A 0xF18A
#define VDP_9938 0x9938
#define VDP_9958 0x9958
#define VDP_9918 0x9918

int detect_vdp();

/*
 * Put the detected VDP into a known state: every control register gets a
 * defined value and all VRAM that ForceCommand can address is cleared. This
 * must run before setupScreen() and must not run while a loaded executable's
 * GPU blob is still sitting in the filesystem file buffer.
 */
void vdp_full_init();

DECLARE_BANKED(detect_vdp, BANK(10), int, bk_detect_vdp, (), ())
DECLARE_BANKED_VOID(vdp_full_init, BANK(10), bk_vdp_full_init, (), ())


#endif
