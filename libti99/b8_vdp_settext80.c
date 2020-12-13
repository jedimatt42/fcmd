#include "vdp.h"

// requires 9938!!
int set_text80_raw() {
	int unblank = VDP_MODE1_UNBLANK | VDP_MODE1_TEXT | VDP_MODE1_INT;

	vdpchar = vdpchar_default;
	scrn_scroll = scrn_scroll_default;

	VDP_SET_REGISTER(VDP_REG_MODE0, VDP_MODE0_80COL);
	VDP_SET_REGISTER(VDP_REG_MODE1, VDP_MODE1_TEXT);
	VDP_SET_REGISTER(0x08, 0x08); // set 64k memory VRAM type
	VDP_SET_REGISTER(0x09, 0x80); // non-interlace, etc. 26.5 row
	VDP_SET_REGISTER(VDP_REG_SIT, 0x03);	gImage = 0x0000;
	VDP_SET_REGISTER(VDP_REG_PDT, 0x02);	gPattern = 0x1000;
	// no sprites and no color in text mode anyway
	nTextRow = 80 * 25;
	nTextEnd = (80 * 26) - 1;
	nTextPos = nTextRow;
	nTextFlags = TEXT_WIDTH_80;

	return unblank;
}

void set_text80() {
    int x = set_text80_raw();
    VDP_SET_REGISTER(VDP_REG_MODE1, x);
    VDP_REG1_KSCAN_MIRROR = x;
}

