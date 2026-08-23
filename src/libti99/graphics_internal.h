#ifndef FORCECOMMAND_GRAPHICS_INTERNAL_H
#define FORCECOMMAND_GRAPHICS_INTERNAL_H 1

#include "graphics.h"
#include "detect_vdp.h"
#include "vdp.h"

extern struct GfxInformation gfx_info;
extern int gfx_foreground;
extern int gfx_background;
extern int gfx_border;
extern int gfx_cursor_x;
extern int gfx_cursor_y;
extern int gfx_sprite_mode;
extern int gfx_sprites_enabled;

#define GFX_VDP_GUARD() __asm__("LIMI 0")

static inline void gfx_vdp_set_address(unsigned int address, int write) {
    if (vdp_type == VDP_9938 || vdp_type == VDP_9958) {
        VDP_SET_REGISTER(0x0e, (unsigned char)(address >> 14));
    }
    if (write) VDP_SET_ADDRESS_WRITE(address & 0x3fff);
    else VDP_SET_ADDRESS(address & 0x3fff);
}

int gfx_is_text_mode(int mode);
int gfx_is_pixel_mode(int mode);
int gfx_is_yjk_mode(int mode);
int gfx_is_paged_mode(int mode);
int gfx_color_valid(int color);
void gfx_set_mode_info(int mode, int width, int height, int physical_width,
                       int physical_height, int colors, int color_model,
                       int capabilities, int page_count);

int gfx_setup_mode(int mode, int sprite_mode, int flags);
int gfx_clear_mode(int color);
int gfx_get_pixel_base(unsigned int base, int x, int y, int* color);
int gfx_set_pixel_base(unsigned int base, int x, int y, int color, int op);

#ifdef MYBANK
DECLARE_BANKED_VOID(gfx_set_mode_info, BANK(14), bk_gfx_set_mode_info,
               (int mode, int width, int height, int physical_width,
                int physical_height, int colors, int color_model,
                int capabilities, int page_count),
               (mode, width, height, physical_width, physical_height, colors,
                color_model, capabilities, page_count))
DECLARE_BANKED(gfx_is_text_mode, BANK(14), int, bk_gfx_is_text_mode,
               (int mode), (mode))
DECLARE_BANKED(gfx_is_yjk_mode, BANK(14), int, bk_gfx_is_yjk_mode,
               (int mode), (mode))
DECLARE_BANKED(gfx_color_valid, BANK(14), int, bk_gfx_color_valid,
               (int color), (color))
DECLARE_BANKED(gfx_setup_mode, BANK(1), int, bk_gfx_setup_mode,
               (int mode, int sprite_mode, int flags),
               (mode, sprite_mode, flags))
DECLARE_BANKED(gfx_clear_mode, BANK(1), int, bk_gfx_clear_mode,
               (int color), (color))
DECLARE_BANKED(gfx_get_pixel_base, BANK(10), int, bk_gfx_get_pixel_base,
               (unsigned int base, int x, int y, int* color),
               (base, x, y, color))
DECLARE_BANKED(gfx_set_pixel_base, BANK(10), int, bk_gfx_set_pixel_base,
               (unsigned int base, int x, int y, int color, int op),
               (base, x, y, color, op))
#endif

#endif
