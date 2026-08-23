#ifndef FORCECOMMAND_GRAPHICS_H
#define FORCECOMMAND_GRAPHICS_H 1

#include "banking.h"

/* High-level graphics modes. Values are an internal ABI detail. */
#define GFX_MODE_GRAPHICS1       0
#define GFX_MODE_TEXT40          1
#define GFX_MODE_GRAPHICS2       2
#define GFX_MODE_BITMAP          GFX_MODE_GRAPHICS2
#define GFX_MODE_MULTICOLOR      3
#define GFX_MODE_GRAPHICS3       4
#define GFX_MODE_GRAPHICS4       5
#define GFX_MODE_GRAPHICS5       6
#define GFX_MODE_GRAPHICS6       7
#define GFX_MODE_GRAPHICS7       8
#define GFX_MODE_TEXT80          9
#define GFX_MODE_YJK_YAE         10
#define GFX_MODE_YJK_RGB         11
#define GFX_MODE_YJK             12
#define GFX_MODE_F18A_TEXT80X30  13

/* Screen setup flags. */
#define GFX_SCREEN_INTERLACED    0x0001

/* Sprite setup values. */
#define GFX_SPRITE_8X8           0x00
#define GFX_SPRITE_8X8_MAG       0x01
#define GFX_SPRITE_16X16         0x02
#define GFX_SPRITE_16X16_MAG     0x03

/* Return values. */
#define GFX_OK                   0
#define GFX_ERR_INVALID          -1
#define GFX_ERR_UNSUPPORTED      -2
#define GFX_ERR_WRONG_MODE       -3
#define GFX_ERR_RANGE            -4
#define GFX_ERR_BUSY             -5
#define GFX_ERR_NO_VRAM          -6

/* Capability bits reported by GfxInformation. */
#define GFX_CAP_TEXT             0x0001
#define GFX_CAP_TILES            0x0002
#define GFX_CAP_PIXELS           0x0004
#define GFX_CAP_LINES            0x0008
#define GFX_CAP_CIRCLES          0x0010
#define GFX_CAP_PAINT            0x0020
#define GFX_CAP_COPY             0x0040
#define GFX_CAP_PAGES            0x0080
#define GFX_CAP_PALETTE          0x0100
#define GFX_CAP_SPRITES          0x0200
#define GFX_CAP_SPRITE_STATUS    0x0400
#define GFX_CAP_YJK              0x0800
#define GFX_CAP_ATTRIBUTES       0x1000

/* Color models. */
#define GFX_COLOR_INDEXED        0
#define GFX_COLOR_DIRECT         1
#define GFX_COLOR_YJK            2
#define GFX_COLOR_ATTRIBUTES     3

/* Drawing operators and styles. */
#define GFX_OP_PSET              0
#define GFX_OP_PRESET            1
#define GFX_OP_AND               2
#define GFX_OP_OR                3
#define GFX_OP_XOR               4
#define GFX_LINE_NORMAL          0x00
#define GFX_LINE_BOX             0x01
#define GFX_LINE_FILL            0x02
#define GFX_COLOR_DEFAULT        -1

#define GFX_RGB(r, g, b) \
    ((((r) & 0x0f) << 8) | (((g) & 0x0f) << 4) | ((b) & 0x0f))

struct GfxInformation {
    int vdp_type;
    int mode;
    int width;
    int height;
    int physical_width;
    int physical_height;
    int colors;
    int color_model;
    int sprite_count;
    int display_page;
    int draw_page;
    int page_count;
    int capabilities;
    unsigned int image_addr;
    unsigned int pattern_addr;
    unsigned int color_addr;
    unsigned int sprite_addr;
    unsigned int sprite_pattern_addr;
};

struct GfxSpriteStatus {
    int collision;
    int fifth_sprite;
    int overflow;
};

int gfx_screen(int mode, int sprite_mode, int flags);
int gfx_get_info(struct GfxInformation* info);
int gfx_color(int foreground, int background, int border);
int gfx_set_page(int display_page, int draw_page);
int gfx_set_cursor(int x, int y);
int gfx_get_cursor(int* x, int* y);
int gfx_clear(int color);
int gfx_pset(int x, int y, int color, int op);
int gfx_preset(int x, int y, int color, int op);
int gfx_point(int x, int y, int* color);
int gfx_line(int x1, int y1, int x2, int y2, int color, int style, int op);
int gfx_circle(int center_x, int center_y, int radius, int color,
              int start_angle, int end_angle, int aspect, int op);
int gfx_paint(int x, int y, int color, int border_color, int op);
int gfx_draw(const char* commands, int color, int op);
int gfx_copy(int source_page, int destination_page, int x1, int y1,
             int x2, int y2, int destination_x, int destination_y, int op);
int gfx_pattern_define(int pattern, const unsigned char* data, int bytes);
int gfx_tile(int x, int y, int pattern, int color);
int gfx_sprite_pattern(int pattern, const unsigned char* data, int bytes);
int gfx_sprite(int number, int pattern, int color, int x, int y);
int gfx_sprite_loc(int number, int x, int y);
int gfx_sprite_hide(int number);
int gfx_sprite_enable(int enabled);
int gfx_sprite_status(struct GfxSpriteStatus* status);
int gfx_palette_set(int index, int red, int green, int blue);

DECLARE_BANKED(gfx_screen, BANK(14), int, bk_gfx_screen, (int mode, int sprite_mode, int flags), (mode, sprite_mode, flags))
DECLARE_BANKED(gfx_get_info, BANK(14), int, bk_gfx_get_info, (struct GfxInformation* info), (info))
DECLARE_BANKED(gfx_color, BANK(14), int, bk_gfx_color, (int foreground, int background, int border), (foreground, background, border))
DECLARE_BANKED(gfx_set_page, BANK(14), int, bk_gfx_set_page, (int display_page, int draw_page), (display_page, draw_page))
DECLARE_BANKED(gfx_set_cursor, BANK(14), int, bk_gfx_set_cursor, (int x, int y), (x, y))
DECLARE_BANKED(gfx_get_cursor, BANK(14), int, bk_gfx_get_cursor, (int* x, int* y), (x, y))
DECLARE_BANKED(gfx_clear, BANK(14), int, bk_gfx_clear, (int color), (color))
DECLARE_BANKED(gfx_pset, BANK(10), int, bk_gfx_pset, (int x, int y, int color, int op), (x, y, color, op))
DECLARE_BANKED(gfx_preset, BANK(10), int, bk_gfx_preset, (int x, int y, int color, int op), (x, y, color, op))
DECLARE_BANKED(gfx_point, BANK(10), int, bk_gfx_point, (int x, int y, int* color), (x, y, color))
DECLARE_BANKED(gfx_line, BANK(13), int, bk_gfx_line, (int x1, int y1, int x2, int y2, int color, int style, int op), (x1, y1, x2, y2, color, style, op))
DECLARE_BANKED(gfx_circle, BANK(13), int, bk_gfx_circle, (int center_x, int center_y, int radius, int color, int start_angle, int end_angle, int aspect, int op), (center_x, center_y, radius, color, start_angle, end_angle, aspect, op))
DECLARE_BANKED(gfx_paint, BANK(13), int, bk_gfx_paint, (int x, int y, int color, int border_color, int op), (x, y, color, border_color, op))
DECLARE_BANKED(gfx_draw, BANK(6), int, bk_gfx_draw, (const char* commands, int color, int op), (commands, color, op))
DECLARE_BANKED(gfx_copy, BANK(6), int, bk_gfx_copy, (int source_page, int destination_page, int x1, int y1, int x2, int y2, int destination_x, int destination_y, int op), (source_page, destination_page, x1, y1, x2, y2, destination_x, destination_y, op))
DECLARE_BANKED(gfx_pattern_define, BANK(9), int, bk_gfx_pattern_define, (int pattern, const unsigned char* data, int bytes), (pattern, data, bytes))
DECLARE_BANKED(gfx_tile, BANK(9), int, bk_gfx_tile, (int x, int y, int pattern, int color), (x, y, pattern, color))
DECLARE_BANKED(gfx_sprite_pattern, BANK(9), int, bk_gfx_sprite_pattern, (int pattern, const unsigned char* data, int bytes), (pattern, data, bytes))
DECLARE_BANKED(gfx_sprite, BANK(9), int, bk_gfx_sprite, (int number, int pattern, int color, int x, int y), (number, pattern, color, x, y))
DECLARE_BANKED(gfx_sprite_loc, BANK(9), int, bk_gfx_sprite_loc, (int number, int x, int y), (number, x, y))
DECLARE_BANKED(gfx_sprite_hide, BANK(9), int, bk_gfx_sprite_hide, (int number), (number))
DECLARE_BANKED(gfx_sprite_enable, BANK(9), int, bk_gfx_sprite_enable, (int enabled), (enabled))
DECLARE_BANKED(gfx_sprite_status, BANK(9), int, bk_gfx_sprite_status, (struct GfxSpriteStatus* status), (status))
DECLARE_BANKED(gfx_palette_set, BANK(14), int, bk_gfx_palette_set, (int index, int red, int green, int blue), (index, red, green, blue))

#endif
