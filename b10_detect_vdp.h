#ifndef _DETECT_VDP_H
#define _DETECT_VDP_H 1

#include "banking.h"

extern int vdp_type;

#define VDP_F18A 0xF18A
#define VDP_9938 0x9938
#define VDP_9958 0x9958
#define VDP_9918 0x9918

int detect_vdp();

DECLARE_BANKED(detect_vdp, BANK(10), int, bk_detect_vdp, (), ())


#endif