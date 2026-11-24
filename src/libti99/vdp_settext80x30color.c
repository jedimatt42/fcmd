#include "banks.h"
#define MYBANK BANK(8)

#include "vdp.h"
#include "vdp_internal.h"
#include "f18a_gpu.h"
#include "gpu_scroll.h"
#include "graphics.h"

// TODO: text modes should not rely on conio support if possible...

extern unsigned int conio_scrnCol; // conio_bgcolor.c

static void vdpchar80color(int pAddr, int ch) {
    VDP_SET_ADDRESS_WRITE(pAddr);
    VDPWD=ch;
    VDP_SET_ADDRESS_WRITE(pAddr-gImage+gColor);
    VDPWD=conio_scrnCol;
}

static void fast_scrn_scroll_80color() {
    // Use the F18A's built-in BLKCPY routine for the row moves and fills.
    bk_gpu_scroll_80color();
}

void set_text80_color(int height)
{
    int x = set_text80_color_raw(height);
    VDP_SET_REGISTER(VDP_REG_MODE1, x);
    VDP_REG1_KSCAN_MIRROR = x;
    vdp_screen_mode_changed(height == 30 ? VDP_SCREENMODE_TEXT80X30
                                        : VDP_SCREENMODE_TEXT80);
}

void set_text80x30_color(void)
{
    set_text80_color(30);
}

// requires F18A!! height is 24 or 30.
int set_text80_color_raw(int height) {
    // unlock the F18A (should be done before setting the mode)
    unlock_f18a();
    if (height == 30) {
        nTextRow = 80 * 29;
        nTextEnd = (80 * 30) - 1;
        nTextFlags = TEXT_FLAG_IS_F18A | TEXT_FLAG_HAS_ATTRIBUTES |
                     TEXT_WIDTH_80 | TEXT_HEIGHT_30;
    } else {
        nTextRow = 80 * 23;
        nTextEnd = (80 * 24) - 1;
        nTextFlags = TEXT_FLAG_IS_F18A | TEXT_FLAG_HAS_ATTRIBUTES |
                     TEXT_WIDTH_80 | TEXT_HEIGHT_24;
    }
    nTextPos = nTextRow;

    int unblank = VDP_MODE1_16K | VDP_MODE1_UNBLANK | VDP_MODE1_TEXT | VDP_MODE1_INT | VDP_MODE1_SPRMODE16x16;
    VDP_SET_REGISTER(VDP_REG_MODE0, VDP_MODE0_80COL);
    VDP_SET_REGISTER(VDP_REG_MODE1, VDP_MODE1_16K | VDP_MODE1_TEXT | VDP_MODE1_SPRMODE16x16);
    VDP_SET_REGISTER(0x31, height == 30 ? 0x40 : 0x00); // row 30 mode
    VDP_SET_REGISTER(0x32, 0x02); // set Position-based tile attributes
    VDP_SET_REGISTER(VDP_REG_SIT, 0x00);
    gImage = 0x000;
    VDP_SET_REGISTER(VDP_REG_PDT, 0x02);
    gPattern = 0x1000;
    VDP_SET_REGISTER(VDP_REG_CT, 0x60);
    gColor = 0x1800;
    VDP_SET_REGISTER(VDP_REG_SAL, 0x14);
    gSprite = 0x0A00;
    VDP_SET_REGISTER(VDP_REG_SDT, 0x01); // sprites can use any of the font patterns.
    gSpritePat = 0x0800; // same as pattern definition table for characters.
    VDP_SET_REGISTER(24 /* palette select */, 0x00);

    vdpchar = vdpchar80color;

    // sprites are active when F18A is unlocked - this turns them all off.
    VDP_SET_REGISTER(0x33, 0x00);
    vdpmemset(gSprite, 0xd0, 128);
    vdpmemset(gColor, conio_scrnCol, nTextEnd + 1);

    // The GPU scroll is a 30-row layout. Use the generic CPU scroll for 24.
    if (height == 30 && bk_f18a_gpu_init() == GFX_OK) {
        scrn_scroll = fast_scrn_scroll_80color;
    } else {
        scrn_scroll = scrn_scroll_default;
    }

    gUnblank = unblank;
    return unblank;
}

int set_text80x30_color_raw() {
    return set_text80_color_raw(30);
}
