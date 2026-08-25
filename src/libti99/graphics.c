#include "banks.h"
#define MYBANK BANK(14)

#include "graphics_internal.h"
#include "detect_vdp.h"
#include "globals.h"
#include "gpu_scroll.h"
#include "palette.h"
#include "terminal.h"
#include "vdp_internal.h"

struct GfxInformation gfx_info = { .mode = -1 };
int gfx_foreground = COLOR_WHITE;
int gfx_background = COLOR_BLACK;
int gfx_border = COLOR_BLACK;
int gfx_cursor_x;
int gfx_cursor_y;
int gfx_sprite_mode = GFX_SPRITE_8X8;
int gfx_sprites_enabled = 1;

static unsigned char gfx_f18a_palette[128];
static int gfx_f18a_palette_ready;
static const unsigned char gfx_default_palette[32] = {
    0x00, 0x00, 0x00, 0x00, 0x02, 0xc3, 0x05, 0xd6,
    0x05, 0x4f, 0x07, 0x6f, 0x0d, 0x54, 0x04, 0xef,
    0x0f, 0x54, 0x0f, 0x76, 0x0d, 0xc3, 0x0e, 0xd6,
    0x02, 0xb2, 0x0c, 0x5c, 0x0c, 0xcc, 0x0f, 0xff
};

int gfx_is_text_mode(int mode) {
    return mode == GFX_MODE_TEXT40 || mode == GFX_MODE_TEXT80 ||
           mode == GFX_MODE_F18A_TEXT80X30;
}

int gfx_is_pixel_mode(int mode) {
    return mode == GFX_MODE_GRAPHICS2 || mode == GFX_MODE_GRAPHICS3 ||
           mode == GFX_MODE_GRAPHICS4 || mode == GFX_MODE_GRAPHICS5 ||
           mode == GFX_MODE_GRAPHICS6 || mode == GFX_MODE_GRAPHICS7 ||
           gfx_is_yjk_mode(mode);
}

int gfx_is_yjk_mode(int mode) {
    return mode == GFX_MODE_YJK_YAE || mode == GFX_MODE_YJK_RGB ||
           mode == GFX_MODE_YJK;
}

int gfx_is_paged_mode(int mode) {
    return mode == GFX_MODE_GRAPHICS4 || mode == GFX_MODE_GRAPHICS5;
}

static int mode_supported(int mode) {
    if (mode == GFX_MODE_GRAPHICS1 || mode == GFX_MODE_TEXT40 ||
        mode == GFX_MODE_GRAPHICS2 || mode == GFX_MODE_MULTICOLOR) {
        return vdp_type == VDP_9918 || vdp_type == VDP_9938 ||
               vdp_type == VDP_9958 || vdp_type == VDP_F18A;
    }
    if (mode == GFX_MODE_TEXT80) {
        return vdp_type == VDP_9938 || vdp_type == VDP_9958 ||
               vdp_type == VDP_F18A;
    }
    if (mode >= GFX_MODE_GRAPHICS3 && mode <= GFX_MODE_GRAPHICS7) {
        return vdp_type == VDP_9938 || vdp_type == VDP_9958;
    }
    if (gfx_is_yjk_mode(mode)) return vdp_type == VDP_9958;
    return mode == GFX_MODE_F18A_TEXT80X30 && vdp_type == VDP_F18A;
}

static int valid_sprite_mode(int mode) {
    return mode == GFX_SPRITE_8X8 || mode == GFX_SPRITE_8X8_MAG ||
           mode == GFX_SPRITE_16X16 || mode == GFX_SPRITE_16X16_MAG;
}

void gfx_set_mode_info(int mode, int width, int height, int physical_width,
                       int physical_height, int colors, int color_model,
                       int capabilities, int page_count) {
    if (vdp_type == VDP_F18A) capabilities |= GFX_CAP_PALETTE;
    gfx_info.vdp_type = vdp_type;
    gfx_info.isPal = pal;
    gfx_info.mode = mode;
    gfx_info.width = width;
    gfx_info.height = height;
    gfx_info.physical_width = physical_width;
    gfx_info.physical_height = physical_height;
    gfx_info.colors = colors;
    gfx_info.color_model = color_model;
    gfx_info.sprite_count = (capabilities & GFX_CAP_SPRITES) ? 32 : 0;
    gfx_info.display_page = 0;
    gfx_info.draw_page = 0;
    gfx_info.page_count = page_count;
    gfx_info.capabilities = capabilities;
    gfx_info.image_addr = gImage;
    gfx_info.pattern_addr = gPattern;
    gfx_info.color_addr = gColor;
    gfx_info.sprite_addr = gSprite;
    gfx_info.sprite_pattern_addr = gSpritePat;
    gfx_cursor_x = 0;
    gfx_cursor_y = 0;
}

int gfx_screen(int mode, int sprite_mode, int flags) {
    if (!valid_sprite_mode(sprite_mode)) return GFX_ERR_INVALID;
    if (mode == GFX_MODE_F18A_TEXT80X30 &&
        !(sprite_mode & GFX_SPRITE_16X16)) return GFX_ERR_UNSUPPORTED;
    if (!mode_supported(mode)) return GFX_ERR_UNSUPPORTED;
    if (flags & ~GFX_SCREEN_INTERLACED) return GFX_ERR_UNSUPPORTED;
    if (flags && !(mode >= GFX_MODE_GRAPHICS3 && mode <= GFX_MODE_YJK)) {
        return GFX_ERR_UNSUPPORTED;
    }
    GFX_VDP_GUARD();
    bk_disable_gpu_scroll();
    int previous_sprite_mode = gfx_sprite_mode;
    gfx_sprite_mode = sprite_mode;
    int result = bk_gfx_setup_mode(mode, sprite_mode, flags);
    if (result != GFX_OK) gfx_sprite_mode = previous_sprite_mode;
    return result;
}

int gfx_get_info(struct GfxInformation* info) {
    if (!info) return GFX_ERR_INVALID;
    if (gfx_info.mode < 0) {
        gfx_info.vdp_type = vdp_type;
        gfx_info.isPal = pal;
        gfx_info.mode = displayWidth == 40 ? GFX_MODE_TEXT40 :
                        (displayHeight == 30 ? GFX_MODE_F18A_TEXT80X30 :
                         GFX_MODE_TEXT80);
        gfx_info.width = displayWidth;
        gfx_info.height = displayHeight;
        gfx_info.physical_width = displayWidth;
        gfx_info.physical_height = displayHeight;
        gfx_info.colors = 16;
        gfx_info.color_model = displayHeight == 30 ? GFX_COLOR_ATTRIBUTES :
                               GFX_COLOR_INDEXED;
        gfx_info.capabilities = GFX_CAP_TEXT;
        if (displayHeight == 30) {
            gfx_info.capabilities |= GFX_CAP_ATTRIBUTES | GFX_CAP_TILES |
                                     GFX_CAP_SPRITES | GFX_CAP_SPRITE_STATUS;
        }
        if (vdp_type == VDP_F18A) gfx_info.capabilities |= GFX_CAP_PALETTE;
        gfx_info.sprite_count = (gfx_info.capabilities & GFX_CAP_SPRITES) ? 32 : 0;
        gfx_info.page_count = 1;
        gfx_info.image_addr = gImage;
        gfx_info.pattern_addr = gPattern;
        gfx_info.color_addr = gColor;
        gfx_info.sprite_addr = gSprite;
        gfx_info.sprite_pattern_addr = gSpritePat;
    }
    *info = gfx_info;
    info->vdp_type = vdp_type;
    info->isPal = pal;
    return GFX_OK;
}

int gfx_color_valid(int color) {
    if (color == GFX_COLOR_DEFAULT) return 1;
    if (gfx_is_yjk_mode(gfx_info.mode)) return color >= 0 && color <= 0x0fff;
    return color >= 0 && color < gfx_info.colors;
}

int gfx_color(int foreground, int background, int border) {
    if (!gfx_color_valid(foreground) || !gfx_color_valid(background) ||
        border < 0 || border > 15) return GFX_ERR_RANGE;
    if (foreground != GFX_COLOR_DEFAULT) gfx_foreground = foreground;
    if (background != GFX_COLOR_DEFAULT) gfx_background = background;
    gfx_border = border;
    GFX_VDP_GUARD();
    if (gfx_is_text_mode(gfx_info.mode)) {
        bk_term_set_text_color(gfx_foreground & 0x0f);
        bk_term_set_bg_color(gfx_background & 0x0f);
    }
    if (gfx_info.mode == GFX_MODE_GRAPHICS1 ||
        gfx_info.mode == GFX_MODE_TEXT40 ||
        gfx_info.mode == GFX_MODE_GRAPHICS2 ||
        gfx_info.mode == GFX_MODE_MULTICOLOR) {
        VDP_SET_REGISTER(VDP_REG_COL, (gfx_foreground << 4) | gfx_background);
    } else if (gfx_info.mode >= GFX_MODE_GRAPHICS3 &&
               gfx_info.mode <= GFX_MODE_GRAPHICS7) {
        VDP_SET_REGISTER(VDP_REG_COL, gfx_border & 0x0f);
    }
    return GFX_OK;
}

int gfx_set_page(int display_page, int draw_page) {
    if (!gfx_is_paged_mode(gfx_info.mode)) return GFX_ERR_UNSUPPORTED;
    if (display_page < 0 || draw_page < 0 ||
        display_page >= gfx_info.page_count ||
        draw_page >= gfx_info.page_count) return GFX_ERR_RANGE;
    GFX_VDP_GUARD();
    if (display_page != draw_page) return GFX_ERR_UNSUPPORTED;
    VDP_SET_REGISTER(VDP_REG_SIT, 0x1f | (display_page << 5));
    gImage = (unsigned int)draw_page * 0x8000;
    gfx_info.image_addr = gImage;
    gfx_info.display_page = display_page;
    gfx_info.draw_page = draw_page;
    bk_vdp_screen_mode_changed_ex(VDP_SCREENMODE_GRAPHICS, 0xec00, 0xed00,
                                  0xee00, 0x1100);
    return GFX_OK;
}

int gfx_set_cursor(int x, int y) {
    if (x < 0 || y < 0 || x >= gfx_info.width || y >= gfx_info.height) {
        return GFX_ERR_RANGE;
    }
    gfx_cursor_x = x;
    gfx_cursor_y = y;
    return GFX_OK;
}

int gfx_get_cursor(int* x, int* y) {
    if (!x || !y) return GFX_ERR_INVALID;
    *x = gfx_cursor_x;
    *y = gfx_cursor_y;
    return GFX_OK;
}

int gfx_clear(int color) {
    if (color == GFX_COLOR_DEFAULT) color = gfx_background;
    if (!gfx_color_valid(color)) return GFX_ERR_RANGE;
    GFX_VDP_GUARD();
    gfx_background = color;
    return bk_gfx_clear_mode(color);
}

static void init_f18a_palette(void) {
    if (gfx_f18a_palette_ready) return;
    int bank;
    for (bank = 0; bank < 4; bank++) {
        int i;
        for (i = 0; i < 32; i++) {
            gfx_f18a_palette[bank * 32 + i] = gfx_default_palette[i];
        }
    }
    gfx_f18a_palette_ready = 1;
}

int gfx_palette_set(int index, int red, int green, int blue) {
    if (vdp_type != VDP_F18A) return GFX_ERR_UNSUPPORTED;
    if (index < 0 || index >= 64 || red < 0 || red > 15 || green < 0 ||
        green > 15 || blue < 0 || blue > 15) return GFX_ERR_RANGE;
    GFX_VDP_GUARD();
    init_f18a_palette();
    unsigned int value = (unsigned int)((red << 8) | (green << 4) | blue);
    gfx_f18a_palette[index * 2] = (unsigned char)(value >> 8);
    gfx_f18a_palette[index * 2 + 1] = (unsigned char)value;
    bk_unlock_f18a();
    VDP_SET_REGISTER(47, 0xc0 | ((index >> 4) << 4));
    int offset = (index >> 4) * 32;
    int i;
    for (i = 0; i < 32; i++) VDPWD = gfx_f18a_palette[offset + i];
    VDP_SET_REGISTER(47, 0);
    if (gfx_info.mode != GFX_MODE_F18A_TEXT80X30) bk_lock_f18a();
    return GFX_OK;
}
