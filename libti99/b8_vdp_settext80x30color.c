#include "banks.h"
#define MYBANK BANK(8)

#include "vdp.h"
#include "b5_gpu_scroll.h"

// TODO: text modes should not rely on conio support if possible...

extern unsigned int conio_scrnCol; // conio_bgcolor.c

static void vdpchar80color(int pAddr, int ch) {
    VDP_SET_ADDRESS_WRITE(pAddr);
    VDPWD=ch;
    VDP_SET_ADDRESS_WRITE(pAddr-gImage+gColor);
    VDPWD=conio_scrnCol;
}

static void fast_scrn_scroll_80color() {
    const int line = nTextEnd - nTextRow + 1;

    // use GPU code for fastest scrolling
    VDP_SET_REGISTER(0x38,1); // trigger GPU code

    VDP_SET_REGISTER(0x0f,2); // status register to read = SR2
    while (VDPST & 0x80);    // wait for GPU status to be idle
    VDP_SET_REGISTER(0x0f,0); // status register to read = SR0

    vdpmemset(nTextRow + gColor, conio_scrnCol, line);  // clear the last line

    return;
}

void set_text80x30_color(void)
{
    int x = set_text80x30_color_raw();
    VDP_SET_REGISTER(VDP_REG_MODE1, x);
    VDP_REG1_KSCAN_MIRROR = x;
}

// requires F18A!!
int set_text80x30_color_raw() {
    // unlock the F18A (should be done before setting the mode)
    unlock_f18a();
    nTextRow = 80 * 29;
    nTextEnd = (80 * 30) - 1;
    nTextPos = nTextRow;
	nTextFlags = TEXT_FLAG_IS_F18A | TEXT_FLAG_HAS_ATTRIBUTES | TEXT_WIDTH_80 | TEXT_HEIGHT_30;

    int unblank = VDP_MODE1_16K | VDP_MODE1_UNBLANK | VDP_MODE1_TEXT | VDP_MODE1_INT;
    VDP_SET_REGISTER(VDP_REG_MODE0, VDP_MODE0_80COL);
    VDP_SET_REGISTER(VDP_REG_MODE1, VDP_MODE1_16K | VDP_MODE1_TEXT);
    VDP_SET_REGISTER(0x31, 0x40); // set 30 row mode
    VDP_SET_REGISTER(0x32, 0x02); // set Position-based tile attributes
    VDP_SET_REGISTER(VDP_REG_SIT, 0x00);
    gImage = 0x000; // to 0x0960
    VDP_SET_REGISTER(VDP_REG_PDT, 0x02);
    gPattern = 0x1000; // to 0x1800
    VDP_SET_REGISTER(VDP_REG_CT, 0x60);
    gColor = 0x1800; // to 0x2160
    VDP_SET_REGISTER(VDP_REG_SAL, 0x20);
    gSprite = 0x0A00; // to 0x0B00
    VDP_SET_REGISTER(VDP_REG_SDT, 0x02); // sprites can use any of the font patterns.
    VDP_SET_REGISTER(24 /* palette select */, 0x00);

    vdpchar = vdpchar80color;
    scrn_scroll = fast_scrn_scroll_80color;

    // sprites are active when F18A is unlocked
    VDP_SET_REGISTER(0x33, 0x00);
    vdpmemset(gSprite, 0xd0, 128);
    vdpmemset(gColor, conio_scrnCol, nTextEnd+1);	// clear the color table

    // load GPU scroll function
    bk_install_gpu_scroll();
    VDP_SET_REGISTER(0x36, 0x3f);
    VDP_SET_REGISTER(0x37, 0x00);

    return unblank;
}


