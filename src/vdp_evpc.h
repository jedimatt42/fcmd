#ifndef _VDP_EVPC_H
#define _VDP_EVPC_H 1

#include "banking.h"

/*
 * EVPC VRAM-page detection: returns non-zero when the VDP lets R14 select a
 * 16K VRAM page, as a V9938/V9958 does.
 */
int vdp_evpc_detect();

/*
 * Apply the EVPC power-up register table and default palette.
 */
void vdp_evpc_preinit();

DECLARE_BANKED(vdp_evpc_detect, BANK(3), int, bk_vdp_evpc_detect, (), ())
DECLARE_BANKED_VOID(vdp_evpc_preinit, BANK(3), bk_vdp_evpc_preinit, (), ())

#endif
