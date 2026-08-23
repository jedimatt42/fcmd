#include "banks.h"
#define MYBANK BANK(8)

#include "vdp.h"
#include "vdp_internal.h"
#include "dsrutil.h"
#include "history.h"
#include "gpu_scroll.h"

void vdp_screen_mode_changed_ex(int mode, unsigned int pab,
                                unsigned int buffer,
                                unsigned int file_buffer,
                                unsigned int file_buffer_size) {
  bk_dsr_set_vdp_buffers(pab, buffer, file_buffer, file_buffer_size);
  bk_history_screen_mode(mode != VDP_SCREENMODE_GRAPHICS);
}

void vdp_screen_mode_changed(int mode) {
	unsigned int buffer = 0x2300;
	unsigned int pab = 0x2160;
	unsigned int file_buffer = 0x2400;
	if (mode == VDP_SCREENMODE_GRAPHICS) {
		// The standard graphics pattern table ends at 0x2800.
		buffer = 0x2800;
		file_buffer = 0x2900;
	}
	vdp_screen_mode_changed_ex(mode, pab, buffer, file_buffer, 0x1100);
}

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
