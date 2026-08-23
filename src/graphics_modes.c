#include "banks.h"
#define MYBANK BANK(1)

#include "graphics_internal.h"
#include "detect_vdp.h"
#include "globals.h"
#include "terminal.h"
#include "vdp_internal.h"

static void set_resources(unsigned int pab, unsigned int buffer,
                          unsigned int file_buffer, unsigned int size) {
    bk_vdp_screen_mode_changed_ex(VDP_SCREENMODE_GRAPHICS, pab, buffer,
                                  file_buffer, size);
}

static void gfx_vdp_memset(unsigned int address, int value, int count) {
    gfx_vdp_set_address(address, 1);
    while (count--) VDPWD = value;
}

static void set_tms_registers(int mode1, int mode0, int sit, int ct,
                              int pdt, int sal, int sdt) {
    VDP_SET_REGISTER(VDP_REG_MODE1, VDP_MODE1_16K);
    VDP_SET_REGISTER(VDP_REG_MODE0, mode0);
    VDP_SET_REGISTER(VDP_REG_SIT, sit);
    VDP_SET_REGISTER(VDP_REG_CT, ct);
    VDP_SET_REGISTER(VDP_REG_PDT, pdt);
    VDP_SET_REGISTER(VDP_REG_SAL, sal);
    VDP_SET_REGISTER(VDP_REG_SDT, sdt);
    VDP_SET_REGISTER(VDP_REG_COL, (gfx_foreground << 4) | gfx_background);
    VDP_SET_REGISTER(VDP_REG_MODE1, mode1);
    VDP_REG1_KSCAN_MIRROR = mode1;
}

static void clear_graphics1(void) {
    vdpmemset(gImage, 0, 768);
    vdpmemset(gColor, (gfx_foreground << 4) | gfx_background, 32);
    vdpmemset(gSprite, 208, 128);
}

static void clear_bitmap(void) {
    vdpmemset(gPattern, 0, 0x1800);
    vdpmemset(gImage, 0, 768);
    vdpmemset(gColor, (gfx_foreground << 4) | gfx_background, 0x1800);
    if (gSprite) vdpmemset(gSprite, 208, 128);
}

static void clear_multicolor(void) {
    vdpmemset(gPattern, (gfx_background << 4) | gfx_background, 0x800);
    vdpmemset(gImage, 0, 768);
    vdpmemset(gSprite, 208, 128);
}

static unsigned int bitmap_bytes(void) {
    if (gfx_info.mode == GFX_MODE_GRAPHICS7 || bk_gfx_is_yjk_mode(gfx_info.mode)) {
        return (unsigned int)gfx_info.width * gfx_info.height;
    }
    if (gfx_info.mode == GFX_MODE_GRAPHICS5) {
        return ((unsigned int)gfx_info.width * gfx_info.height) >> 2;
    }
    return ((unsigned int)gfx_info.width * gfx_info.height) >> 1;
}

static unsigned char packed_color(int color) {
    if (gfx_info.mode == GFX_MODE_GRAPHICS5) {
        color &= 3;
        return (unsigned char)((color << 6) | (color << 4) |
                               (color << 2) | color);
    }
    if (gfx_info.mode == GFX_MODE_GRAPHICS4 ||
        gfx_info.mode == GFX_MODE_GRAPHICS6) {
        return (unsigned char)((color << 4) | color);
    }
    return (unsigned char)color;
}

static void clear_yamaha(void) {
    gfx_vdp_memset(gImage, packed_color(gfx_background), bitmap_bytes());
}

static int setup_graphics1(void) {
    set_tms_registers(VDP_MODE1_16K | VDP_MODE1_UNBLANK |
                      VDP_MODE1_INT | gfx_sprite_mode, 0, 0, 0x0e, 1, 6, 1);
    gUnblank = VDP_MODE1_16K | VDP_MODE1_UNBLANK |
               VDP_MODE1_INT | gfx_sprite_mode;
    gImage = 0;
    gColor = 0x380;
    gPattern = 0x800;
    gSprite = 0x300;
    gSpritePat = 0x800;
    nTextFlags = TEXT_WIDTH_32;
    displayWidth = 32;
    displayHeight = 24;
    clear_graphics1();
    set_resources(0x3a00, 0x2800, 0x2900, 0x0c00);
    bk_gfx_set_mode_info(GFX_MODE_GRAPHICS1, 32, 24, 256, 192, 16,
                         GFX_COLOR_INDEXED, GFX_CAP_TILES | GFX_CAP_SPRITES |
                         GFX_CAP_SPRITE_STATUS, 1);
    return GFX_OK;
}

static int setup_text40(void) {
    bk_vdp_screenmode(VDP_SCREENMODE_TEXT);
    displayWidth = 40;
    displayHeight = 24;
    bk_gfx_set_mode_info(GFX_MODE_TEXT40, 40, 24, 40, 24, 16,
                         GFX_COLOR_INDEXED, GFX_CAP_TEXT, 1);
    bk_term_cls();
    return GFX_OK;
}

static int setup_graphics2(void) {
    set_tms_registers(VDP_MODE1_16K | VDP_MODE1_UNBLANK |
                      VDP_MODE1_INT | gfx_sprite_mode, VDP_MODE0_BITMAP,
                      6, 0xff, 3, 0x36, 0);
    gUnblank = VDP_MODE1_16K | VDP_MODE1_UNBLANK |
               VDP_MODE1_INT | gfx_sprite_mode;
    gImage = 0x1800;
    gColor = 0x2000;
    gPattern = 0;
    gSprite = 0x1b00;
    gSpritePat = 0;
    nTextFlags = TEXT_FLAG_IS_BITMAPPED;
    displayWidth = 256;
    displayHeight = 192;
    clear_bitmap();
    set_resources(0x1d00, 0x1c00, 0x1e00, 0x0200);
    bk_gfx_set_mode_info(GFX_MODE_GRAPHICS2, 256, 192, 256, 192, 16,
                         GFX_COLOR_INDEXED, GFX_CAP_PIXELS | GFX_CAP_LINES |
                         GFX_CAP_CIRCLES | GFX_CAP_PAINT | GFX_CAP_SPRITES |
                         GFX_CAP_SPRITE_STATUS, 1);
    return GFX_OK;
}

static int setup_multicolor(void) {
    set_tms_registers(VDP_MODE1_16K | VDP_MODE1_UNBLANK | VDP_MODE1_INT |
                      VDP_MODE1_MULTI | gfx_sprite_mode, 0, 2, 0, 0, 0x36, 0);
    gUnblank = VDP_MODE1_16K | VDP_MODE1_UNBLANK | VDP_MODE1_INT |
               VDP_MODE1_MULTI | gfx_sprite_mode;
    gImage = 0x800;
    gColor = 0;
    gPattern = 0;
    gSprite = 0x1b00;
    gSpritePat = 0;
    nTextFlags = TEXT_FLAG_IS_MULTICOLOR;
    displayWidth = 64;
    displayHeight = 48;
    clear_multicolor();
    set_resources(0x3b00, 0x2800, 0x2900, 0x0c00);
    bk_gfx_set_mode_info(GFX_MODE_MULTICOLOR, 64, 48, 256, 192, 16,
                         GFX_COLOR_INDEXED, GFX_CAP_PIXELS | GFX_CAP_LINES |
                         GFX_CAP_CIRCLES | GFX_CAP_PAINT | GFX_CAP_SPRITES |
                         GFX_CAP_SPRITE_STATUS, 1);
    return GFX_OK;
}

static int setup_text80(void) {
    bk_vdp_screenmode(VDP_SCREENMODE_TEXT80);
#ifdef _CLASSIC_99
    displayHeight = 24;
#else
    displayHeight = 26;
#endif
    displayWidth = 80;
    bk_gfx_set_mode_info(GFX_MODE_TEXT80, displayWidth, displayHeight,
                         displayWidth, displayHeight, 16, GFX_COLOR_INDEXED,
                         GFX_CAP_TEXT, 1);
    bk_term_cls();
    return GFX_OK;
}

static int setup_text80x30(void) {
    bk_vdp_screenmode(VDP_SCREENMODE_TEXT80X30);
    displayWidth = 80;
    displayHeight = 30;
    bk_gfx_set_mode_info(GFX_MODE_F18A_TEXT80X30, 80, 30, 80, 30, 16,
                         GFX_COLOR_ATTRIBUTES, GFX_CAP_TEXT | GFX_CAP_ATTRIBUTES |
                         GFX_CAP_TILES | GFX_CAP_SPRITES | GFX_CAP_SPRITE_STATUS,
                         1);
    bk_term_cls();
    return GFX_OK;
}

static int setup_yamaha_bitmap(int mode, int flags) {
    int mode0;
    int width;
    int height = 212;
    int colors;
    int color_model;
    int page_count = 1;
    unsigned int buffer;
    unsigned int file_buffer;
    unsigned int file_buffer_size;
    int sit = 0x1f;
    int sprite_attr = 0;
    int sprite_pattern = 0;
    int capabilities = GFX_CAP_PIXELS | GFX_CAP_LINES | GFX_CAP_CIRCLES |
                       GFX_CAP_PAINT | GFX_CAP_COPY;
    if (flags) return GFX_ERR_UNSUPPORTED;
    switch (mode) {
    case GFX_MODE_GRAPHICS3:
        gImage = 0x1800;
        gColor = 0x2000;
        gPattern = 0;
        mode0 = 4; width = 256; height = 192; colors = 16;
        color_model = GFX_COLOR_INDEXED;
        buffer = 0x1c00; file_buffer = 0x1e00; file_buffer_size = 0x200;
        capabilities &= ~GFX_CAP_COPY;
        break;
    case GFX_MODE_GRAPHICS4:
        mode0 = 6; width = 256; colors = 16;
        color_model = GFX_COLOR_INDEXED;
        buffer = 0xef00; file_buffer = 0xf000; file_buffer_size = 0xe00;
        page_count = 2; capabilities |= GFX_CAP_PAGES;
        sprite_attr = 0xf4; sprite_pattern = 0x0e;
        capabilities |= GFX_CAP_SPRITES | GFX_CAP_SPRITE_STATUS;
        break;
    case GFX_MODE_GRAPHICS5:
        mode0 = 8; width = 512; colors = 4;
        color_model = GFX_COLOR_INDEXED;
        buffer = 0xef00; file_buffer = 0xf000; file_buffer_size = 0xe00;
        page_count = 2; capabilities |= GFX_CAP_PAGES;
        sprite_attr = 0xf4; sprite_pattern = 0x0e;
        capabilities |= GFX_CAP_SPRITES | GFX_CAP_SPRITE_STATUS;
        break;
    case GFX_MODE_GRAPHICS6:
        mode0 = 10; width = 512; colors = 16;
        color_model = GFX_COLOR_INDEXED;
        buffer = 0xd500; file_buffer = 0xd600; file_buffer_size = 0x2600;
        sit = 0x3f;
        sprite_attr = 0xf8; sprite_pattern = 0x1e;
        break;
    case GFX_MODE_GRAPHICS7:
        mode0 = 14; width = 256; colors = 256;
        color_model = GFX_COLOR_DIRECT;
        buffer = 0xd500; file_buffer = 0xd600; file_buffer_size = 0x2600;
        sit = 0x3f;
        sprite_attr = 0xf8; sprite_pattern = 0x1e;
        break;
    default:
        return GFX_ERR_INVALID;
    }
    VDP_SET_REGISTER(VDP_REG_MODE1, VDP_MODE1_TEXT);
    VDP_SET_REGISTER(VDP_REG_MODE0, mode0);
    int mode1 = VDP_MODE1_UNBLANK | VDP_MODE1_INT |
                (gfx_sprite_mode & (VDP_MODE1_SPRMODE16x16 | VDP_MODE1_SPRMAG));
    VDP_SET_REGISTER(VDP_REG_MODE1, mode1);
    if (mode == GFX_MODE_GRAPHICS3) {
        VDP_SET_REGISTER(VDP_REG_SIT, 6);
        VDP_SET_REGISTER(VDP_REG_CT, 0xff);
        VDP_SET_REGISTER(VDP_REG_PDT, 3);
    } else {
        VDP_SET_REGISTER(VDP_REG_SIT, sit);
        VDP_SET_REGISTER(VDP_REG_CT, 0);
        VDP_SET_REGISTER(VDP_REG_PDT, 0);
    }
    VDP_SET_REGISTER(VDP_REG_SAL, sprite_attr);
    VDP_SET_REGISTER(VDP_REG_SDT, sprite_pattern);
    VDP_SET_REGISTER(8, 8);
    VDP_SET_REGISTER(9, height == 212 ? 0x80 : 0);
    VDP_SET_REGISTER(VDP_REG_COL, gfx_border & 0x0f);
    gUnblank = mode1;
    if (mode != GFX_MODE_GRAPHICS3) {
        gImage = 0;
        gPattern = 0;
        gColor = 0;
    }
    gSprite = sprite_attr == 0 ? 0 :
              (mode == GFX_MODE_GRAPHICS4 || mode == GFX_MODE_GRAPHICS5 ?
               0x7a00 : 0xfc00);
    gSpritePat = sprite_pattern == 0 ? 0 :
                 (mode == GFX_MODE_GRAPHICS4 || mode == GFX_MODE_GRAPHICS5 ?
                  0x7000 : 0xf000);
    nTextFlags = TEXT_FLAG_IS_BITMAPPED;
    displayWidth = width;
    displayHeight = height;
    bk_gfx_set_mode_info(mode, width, height, width, height, colors,
                         color_model, capabilities, page_count);
    if (mode == GFX_MODE_GRAPHICS3) clear_bitmap();
    else clear_yamaha();
    if (gSprite) gfx_vdp_memset(gSprite, 208, 128);
    set_resources(mode == GFX_MODE_GRAPHICS3 ? 0x1d00 :
                  (mode == GFX_MODE_GRAPHICS6 || mode == GFX_MODE_GRAPHICS7 ?
                   0xd400 : 0xec00),
                  buffer, file_buffer, file_buffer_size);
    return GFX_OK;
}

static int setup_yjk(int mode, int flags) {
    int result = setup_yamaha_bitmap(GFX_MODE_GRAPHICS7, flags);
    if (result != GFX_OK) return result;
    VDP_SET_REGISTER(25, mode == GFX_MODE_YJK_YAE ? 0x18 : 0x08);
    gfx_info.mode = mode;
    gfx_info.colors = 0;
    gfx_info.color_model = GFX_COLOR_YJK;
    gfx_info.capabilities |= GFX_CAP_YJK;
    return GFX_OK;
}

int gfx_setup_mode(int mode, int sprite_mode, int flags) {
    (void)sprite_mode;
    GFX_VDP_GUARD();
    if (vdp_type == VDP_9938 || vdp_type == VDP_9958) {
        VDP_SET_REGISTER(0x0e, 0);
    }
    if (vdp_type == VDP_F18A && mode != GFX_MODE_F18A_TEXT80X30) {
        bk_lock_f18a();
    }
    if (mode == GFX_MODE_GRAPHICS1) return setup_graphics1();
    if (mode == GFX_MODE_TEXT40) return setup_text40();
    if (mode == GFX_MODE_GRAPHICS2) return setup_graphics2();
    if (mode == GFX_MODE_MULTICOLOR) return setup_multicolor();
    if (mode == GFX_MODE_TEXT80) return setup_text80();
    if (mode == GFX_MODE_F18A_TEXT80X30) return setup_text80x30();
    if (bk_gfx_is_yjk_mode(mode)) return setup_yjk(mode, flags);
    return setup_yamaha_bitmap(mode, flags);
}

int gfx_clear_mode(int color) {
    GFX_VDP_GUARD();
    if (bk_gfx_is_text_mode(gfx_info.mode)) {
        bk_term_cls();
        return GFX_OK;
    }
    if (gfx_info.mode == GFX_MODE_GRAPHICS1) {
        gfx_background = color;
        clear_graphics1();
    } else if (gfx_info.mode == GFX_MODE_GRAPHICS2 ||
               gfx_info.mode == GFX_MODE_GRAPHICS3) {
        gfx_background = color;
        clear_bitmap();
    } else if (gfx_info.mode == GFX_MODE_MULTICOLOR) {
        gfx_background = color;
        clear_multicolor();
    } else if (bk_gfx_is_yjk_mode(gfx_info.mode)) {
        return GFX_ERR_UNSUPPORTED;
    } else {
        gfx_vdp_memset(gImage, packed_color(color), bitmap_bytes());
    }
    return GFX_OK;
}
