#include "banks.h"
#define MYBANK BANK(8)

#include "vdp.h"
#include "gpu_scroll.h"

int vdp_screenmode(int mode) {
	bk_disable_gpu_scroll();
	switch (mode) {
	case VDP_SCREENMODE_GRAPHICS:
		set_graphics(VDP_SPR_8x8);
		return gUnblank;
	case VDP_SCREENMODE_TEXT:
		set_text();
		return gUnblank;
	case VDP_SCREENMODE_TEXT80:
		set_text80();
		return gUnblank;
	case VDP_SCREENMODE_TEXT80X30:
		set_text80x30_color();
		return gUnblank;
	}
	return gUnblank;
}
