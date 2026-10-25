#include "banks.h"
#define MYBANK BANK(14)

#include "graphics_internal.h"

#define GFX_MAX_SPRITES 32

static void vram_write(unsigned int address, unsigned char value) {
    gfx_vdp_set_address(address, 1);
    VDPWD = value;
}

static int sprite_number_valid(int number) {
    if (!(gfx_info.capabilities & GFX_CAP_SPRITES)) return GFX_ERR_WRONG_MODE;
    if (number < 0 || number >= GFX_MAX_SPRITES) return GFX_ERR_RANGE;
    return GFX_OK;
}

int gfx_sprite_color(int number, int color) {
    GFX_VDP_GUARD();
    int result = sprite_number_valid(number);
    if (result != GFX_OK) return result;
    if (color < 0 || color > 15) return GFX_ERR_RANGE;
    vram_write(gSprite + (number << 2) + 3, (unsigned char)color);
    return GFX_OK;
}

int gfx_sprite_pattern_index(int number, int pattern) {
    GFX_VDP_GUARD();
    int result = sprite_number_valid(number);
    if (result != GFX_OK) return result;
    int pattern_count = (gfx_sprite_mode & GFX_SPRITE_16X16) ? 64 : 256;
    if (pattern < 0 || pattern >= pattern_count) return GFX_ERR_RANGE;
    vram_write(gSprite + (number << 2) + 2, (unsigned char)pattern);
    return GFX_OK;
}
