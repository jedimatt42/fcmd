#include "banks.h"
#define MYBANK BANK(8)

#include "vdp.h"
#include "vdp_internal.h"
#include "detect_vdp.h"

// requires 9938!! height is 24 or 26.
int set_text80_raw_height(int height) {
	int unblank = VDP_MODE1_UNBLANK | VDP_MODE1_TEXT | VDP_MODE1_INT;

	vdpchar = vdpchar_default;
	scrn_scroll = scrn_scroll_default;

	// Set the 80-column (M4) bit before writing VR8/VR9. On a locked F18A,
	// writes above VR7 are masked until M4 is set, so the earlier order lost
	// the 64K/26.5-row setup and TEXT80 rendered blank.
	VDP_SET_REGISTER(VDP_REG_MODE0, VDP_MODE0_80COL);
	VDP_SET_REGISTER(VDP_REG_MODE1, VDP_MODE1_TEXT);
#ifndef _CLASSIC_99
	// VR8/VR9 are ignored while a locked F18A masks writes above VR7, so
	// unlock for these two, then re-lock (which also keeps the T80 name-table
	// low bits masked off, matching gImage = 0).
	if (vdp_type == VDP_F18A) unlock_f18a();
	VDP_SET_REGISTER(0x08, 0x08); // set 64k memory VRAM type
	VDP_SET_REGISTER(0x09, height == 26 ? 0x80 : 0x00);
	if (vdp_type == VDP_F18A) VDP_SET_REGISTER(0x39, 0x00);
#endif
	VDP_SET_REGISTER(VDP_REG_SIT, 0x03);	gImage = 0x0000;
	VDP_SET_REGISTER(VDP_REG_PDT, 0x02);	gPattern = 0x1000;
	if (height == 26) {
		nTextRow = 80 * 25;
		nTextEnd = (80 * 26) - 1;
		nTextFlags = TEXT_WIDTH_80 | TEXT_HEIGHT_26;
	} else {
		nTextRow = 80 * 23;
		nTextEnd = (80 * 24) - 1;
		nTextFlags = TEXT_WIDTH_80 | TEXT_HEIGHT_24;
	}
	nTextPos = nTextRow;

	gUnblank = unblank;
	return unblank;
}

int set_text80_raw() {
	return set_text80_raw_height(26);
}

void set_text80_height(int height) {
    int x = set_text80_raw_height(height);
    VDP_SET_REGISTER(VDP_REG_MODE1, x);
    VDP_REG1_KSCAN_MIRROR = x;
    vdp_screen_mode_changed(VDP_SCREENMODE_TEXT80);
}

void set_text80() {
    set_text80_height(26);
}
