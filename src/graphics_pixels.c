#include "banks.h"
#define MYBANK BANK(10)

#include "graphics_internal.h"

static unsigned char vram_read(unsigned int address) {
    gfx_vdp_set_address(address, 0);
    __asm__("NOP");
    return VDPRD;
}

static void vram_write(unsigned int address, unsigned char value) {
    gfx_vdp_set_address(address, 1);
    VDPWD = value;
}

static int pixel_address(unsigned int base, int x, int y,
                         unsigned int* address, unsigned char* mask,
                         int* shift) {
    int mode = gfx_info.mode;
    if (mode == GFX_MODE_GRAPHICS2 || mode == GFX_MODE_GRAPHICS3) {
        unsigned int name = gImage + (unsigned int)(y >> 3) * 32 + (x >> 3);
        unsigned int pattern = vram_read(name);
        unsigned int offset = ((unsigned int)(y >> 6) << 8) +
                              (pattern << 3) + (y & 7);
        *address = gPattern + offset;
        *mask = (unsigned char)(0x80 >> (x & 7));
        *shift = 0;
        return GFX_OK;
    }
    if (mode == GFX_MODE_MULTICOLOR) {
        int row = y >> 1;
        int col = x >> 1;
        int pattern_row = row >> 2;
        unsigned int name = gImage + (unsigned int)pattern_row * 32 + col;
        unsigned int pattern = vram_read(name);
        *address = gPattern + (pattern << 3) + ((row & 3) << 1) + (y & 1);
        *mask = 0xf0;
        *shift = (x & 1) ? 0 : 4;
        return GFX_OK;
    }
    if (mode == GFX_MODE_GRAPHICS4 || mode == GFX_MODE_GRAPHICS6) {
        *address = base + (unsigned int)y * (gfx_info.width >> 1) + (x >> 1);
        *mask = (x & 1) ? 0x0f : 0xf0;
        *shift = (x & 1) ? 0 : 4;
        return GFX_OK;
    }
    if (mode == GFX_MODE_GRAPHICS5) {
        *address = base + (unsigned int)y * (gfx_info.width >> 2) + (x >> 2);
        *mask = (unsigned char)(3 << ((3 - (x & 3)) << 1));
        *shift = (3 - (x & 3)) << 1;
        return GFX_OK;
    }
    if (mode == GFX_MODE_GRAPHICS7 || bk_gfx_is_yjk_mode(mode)) {
        *address = base + (unsigned int)y * gfx_info.width + x;
        *mask = 0xff;
        *shift = 0;
        return GFX_OK;
    }
    return GFX_ERR_WRONG_MODE;
}

static int yjk_pixel(unsigned int base, int x, int y, int* color) {
    unsigned int group = base + (unsigned int)y * gfx_info.width + (x & ~3);
    unsigned char bytes[4];
    int i;
    for (i = 0; i < 4; i++) bytes[i] = vram_read(group + i);
    int j = ((bytes[2] & 7) << 3) | (bytes[3] & 7);
    int k = ((bytes[0] & 7) << 3) | (bytes[1] & 7);
    if (j & 0x20) j -= 64;
    if (k & 0x20) k -= 64;
    int y_value = bytes[x & 3] >> 3;
    int red = y_value + j;
    int green = y_value + k;
    int blue = (5 * y_value - 2 * j - k) / 4;
    if (red < 0) red = 0; if (red > 63) red = 63;
    if (green < 0) green = 0; if (green > 63) green = 63;
    if (blue < 0) blue = 0; if (blue > 63) blue = 63;
    *color = ((red * 15 / 63) << 8) | ((green * 15 / 63) << 4) |
             (blue * 15 / 63);
    return GFX_OK;
}

int gfx_get_pixel_base(unsigned int base, int x, int y, int* color) {
    GFX_VDP_GUARD();
    unsigned int address;
    unsigned char mask;
    int shift;
    if (bk_gfx_is_yjk_mode(gfx_info.mode)) return yjk_pixel(base, x, y, color);
    if (pixel_address(base, x, y, &address, &mask, &shift) != GFX_OK) {
        return GFX_ERR_WRONG_MODE;
    }
    unsigned char value = vram_read(address);
    *color = (value & mask) >> shift;
    if (gfx_info.mode == GFX_MODE_GRAPHICS2 ||
        gfx_info.mode == GFX_MODE_GRAPHICS3) {
        unsigned char color_byte = vram_read(gColor + (address - gPattern));
        *color = (value & mask) ? (color_byte >> 4) : (color_byte & 0x0f);
    }
    return GFX_OK;
}

static int apply_op(int old_color, int color, int op) {
    switch (op) {
    case GFX_OP_PSET:
    case GFX_OP_PRESET: return color;
    case GFX_OP_AND: return old_color & color;
    case GFX_OP_OR: return old_color | color;
    case GFX_OP_XOR: return old_color ^ color;
    default: return -1;
    }
}

static int set_yjk_pixel(unsigned int base, int x, int y, int color) {
    int red = ((color >> 8) & 0x0f) * 4;
    int green = ((color >> 4) & 0x0f) * 4;
    int blue = (color & 0x0f) * 4;
    int y_value = (4 * blue + 2 * red + green) / 8;
    int j = red - y_value;
    int k = green - y_value;
    if (y_value < 0) y_value = 0;
    if (y_value > 31) y_value = 31;
    if (j < -32) j = -32; if (j > 31) j = 31;
    if (k < -32) k = -32; if (k > 31) k = 31;
    unsigned int group = base + (unsigned int)y * gfx_info.width + (x & ~3);
    unsigned char bytes[4];
    int i;
    for (i = 0; i < 4; i++) bytes[i] = vram_read(group + i) & 0xf8;
    bytes[0] |= k & 7;
    bytes[1] |= (k >> 3) & 7;
    bytes[2] |= j & 7;
    bytes[3] |= (j >> 3) & 7;
    bytes[x & 3] = (unsigned char)((y_value << 3) | (bytes[x & 3] & 7));
    for (i = 0; i < 4; i++) vram_write(group + i, bytes[i]);
    return GFX_OK;
}

int gfx_set_pixel_base(unsigned int base, int x, int y, int color, int op) {
    GFX_VDP_GUARD();
    int old_color;
    int result = gfx_get_pixel_base(base, x, y, &old_color);
    if (result != GFX_OK) return result;
    int new_color = apply_op(old_color, color, op);
    if (new_color < 0) return GFX_ERR_INVALID;
    if (!bk_gfx_color_valid(new_color)) return GFX_ERR_RANGE;
    if (bk_gfx_is_yjk_mode(gfx_info.mode)) {
        if (op != GFX_OP_PSET && op != GFX_OP_PRESET) return GFX_ERR_UNSUPPORTED;
        return set_yjk_pixel(base, x, y, new_color);
    }
    unsigned int address;
    unsigned char mask;
    int shift;
    pixel_address(base, x, y, &address, &mask, &shift);
    unsigned char value = vram_read(address);
    if (gfx_info.mode == GFX_MODE_GRAPHICS2 ||
        gfx_info.mode == GFX_MODE_GRAPHICS3) {
        unsigned int color_address = gColor + (address - gPattern);
        unsigned char color_byte = vram_read(color_address);
        if (new_color == gfx_background) {
            value &= (unsigned char)~mask;
        } else {
            value |= mask;
            color_byte = (unsigned char)((new_color << 4) | (color_byte & 0x0f));
            vram_write(color_address, color_byte);
        }
        vram_write(address, value);
    } else {
        value = (unsigned char)((value & ~mask) | ((new_color << shift) & mask));
        vram_write(address, value);
    }
    return GFX_OK;
}

static int check_pixel(int x, int y) {
    if (x < 0 || y < 0 || x >= gfx_info.width || y >= gfx_info.height) {
        return GFX_ERR_RANGE;
    }
    return (gfx_info.capabilities & GFX_CAP_PIXELS) ? GFX_OK : GFX_ERR_WRONG_MODE;
}

int gfx_pset(int x, int y, int color, int op) {
    int result = check_pixel(x, y);
    if (result != GFX_OK) return result;
    if (color == GFX_COLOR_DEFAULT) color = gfx_foreground;
    if (!bk_gfx_color_valid(color)) return GFX_ERR_RANGE;
    result = gfx_set_pixel_base(gImage, x, y, color, op);
    if (result == GFX_OK) { gfx_cursor_x = x; gfx_cursor_y = y; }
    return result;
}

int gfx_preset(int x, int y, int color, int op) {
    int result = check_pixel(x, y);
    if (result != GFX_OK) return result;
    if (color == GFX_COLOR_DEFAULT) color = gfx_background;
    if (!bk_gfx_color_valid(color)) return GFX_ERR_RANGE;
    result = gfx_set_pixel_base(gImage, x, y, color, op);
    if (result == GFX_OK) { gfx_cursor_x = x; gfx_cursor_y = y; }
    return result;
}

int gfx_point(int x, int y, int* color) {
    int result = check_pixel(x, y);
    if (result != GFX_OK) return result;
    if (!color) return GFX_ERR_INVALID;
    return gfx_get_pixel_base(gImage, x, y, color);
}
