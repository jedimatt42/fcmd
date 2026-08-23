#include "banks.h"
#define MYBANK BANK(9)

#include "graphics_internal.h"

#define GFX_MAX_SPRITES 32

static unsigned char vram_read(unsigned int address) {
    gfx_vdp_set_address(address, 0);
    __asm__("NOP");
    return VDPRD;
}

static void vram_write(unsigned int address, unsigned char value) {
    gfx_vdp_set_address(address, 1);
    VDPWD = value;
}

int gfx_pattern_define(int pattern, const unsigned char* data, int bytes) {
    GFX_VDP_GUARD();
    if (!(gfx_info.capabilities & GFX_CAP_TILES)) return GFX_ERR_WRONG_MODE;
    if (!data || bytes != 8 || pattern < 0 || pattern >= 256) {
        return GFX_ERR_INVALID;
    }
    vdpmemcpy(gPattern + (pattern << 3), (const char*)data, bytes);
    return GFX_OK;
}

int gfx_tile(int x, int y, int pattern, int color) {
    GFX_VDP_GUARD();
    if (!(gfx_info.capabilities & GFX_CAP_TILES)) return GFX_ERR_WRONG_MODE;
    if (gfx_info.mode != GFX_MODE_GRAPHICS1 &&
        gfx_info.mode != GFX_MODE_F18A_TEXT80X30) return GFX_ERR_WRONG_MODE;
    int width = gfx_info.mode == GFX_MODE_GRAPHICS1 ? 32 : 80;
    int height = gfx_info.mode == GFX_MODE_GRAPHICS1 ? 24 : 30;
    if (x < 0 || y < 0 || x >= width || y >= height || pattern < 0 ||
        pattern >= 256 || !bk_gfx_color_valid(color)) return GFX_ERR_RANGE;
    vram_write(gImage + (unsigned int)y * width + x, (unsigned char)pattern);
    if (gfx_info.mode == GFX_MODE_GRAPHICS1) {
        vram_write(gColor + (pattern >> 3),
                   (unsigned char)((color << 4) | gfx_background));
    } else {
        vram_write(gColor + (unsigned int)y * 80 + x, (unsigned char)color);
    }
    return GFX_OK;
}

int gfx_sprite_pattern(int pattern, const unsigned char* data, int bytes) {
    GFX_VDP_GUARD();
    if (!(gfx_info.capabilities & GFX_CAP_SPRITES)) return GFX_ERR_WRONG_MODE;
    int expected = (gfx_sprite_mode & GFX_SPRITE_16X16) ? 32 : 8;
    int pattern_count = expected == 32 ? 64 : 256;
    if (!data || bytes != expected || pattern < 0 || pattern >= pattern_count) {
        return GFX_ERR_INVALID;
    }
    vdpmemcpy(gSpritePat + pattern * expected, (const char*)data, bytes);
    return GFX_OK;
}

static int sprite_valid(int number, int x, int y) {
    if (!(gfx_info.capabilities & GFX_CAP_SPRITES)) return GFX_ERR_WRONG_MODE;
    if (number < 0 || number >= GFX_MAX_SPRITES) return GFX_ERR_RANGE;
    if (x < 0 || x > 255 || y < 0 || y > 255) return GFX_ERR_RANGE;
    return GFX_OK;
}

int gfx_sprite(int number, int pattern, int color, int x, int y) {
    GFX_VDP_GUARD();
    int result = sprite_valid(number, x, y);
    if (result != GFX_OK) return result;
    int pattern_count = (gfx_sprite_mode & GFX_SPRITE_16X16) ? 64 : 256;
    if (pattern < 0 || pattern >= pattern_count || color < 0 || color > 15) {
        return GFX_ERR_RANGE;
    }
    unsigned int address = gSprite + (number << 2);
    vram_write(address, (unsigned char)y);
    vram_write(address + 1, (unsigned char)x);
    vram_write(address + 2, (unsigned char)pattern);
    vram_write(address + 3, (unsigned char)color);
    return GFX_OK;
}

int gfx_sprite_loc(int number, int x, int y) {
    GFX_VDP_GUARD();
    int result = sprite_valid(number, x, y);
    if (result != GFX_OK) return result;
    unsigned int address = gSprite + (number << 2);
    vram_write(address, (unsigned char)y);
    vram_write(address + 1, (unsigned char)x);
    return GFX_OK;
}

int gfx_sprite_hide(int number) {
    GFX_VDP_GUARD();
    if (!(gfx_info.capabilities & GFX_CAP_SPRITES)) return GFX_ERR_WRONG_MODE;
    if (number < 0 || number >= GFX_MAX_SPRITES) return GFX_ERR_RANGE;
    vram_write(gSprite + (number << 2), 208);
    return GFX_OK;
}

int gfx_sprite_enable(int enabled) {
    GFX_VDP_GUARD();
    if (!(gfx_info.capabilities & GFX_CAP_SPRITES)) return GFX_ERR_WRONG_MODE;
    gfx_sprites_enabled = enabled != 0;
    if (gfx_info.mode == GFX_MODE_F18A_TEXT80X30) {
        VDP_SET_REGISTER(0x33, gfx_sprites_enabled ? 0xff : 0x00);
    }
    if (!gfx_sprites_enabled) {
        int i;
        for (i = 0; i < GFX_MAX_SPRITES; i++) gfx_sprite_hide(i);
    }
    return GFX_OK;
}

int gfx_sprite_status(struct GfxSpriteStatus* status) {
    GFX_VDP_GUARD();
    if (!status) return GFX_ERR_INVALID;
    if (!(gfx_info.capabilities & GFX_CAP_SPRITE_STATUS)) {
        return GFX_ERR_WRONG_MODE;
    }
    VDP_SET_REGISTER(0x0f, 0);
    unsigned char value = VDPST;
    status->collision = (value & VDP_ST_COINC) != 0;
    status->overflow = (value & VDP_ST_5SP) != 0;
    status->fifth_sprite = value & VDP_ST_MASK;
    return GFX_OK;
}
