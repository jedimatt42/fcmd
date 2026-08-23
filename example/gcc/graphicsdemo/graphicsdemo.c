#include <fc_api.h>

static const unsigned char ball_pattern[8] = {
    0x18, 0x3c, 0x7e, 0xff, 0xff, 0x7e, 0x3c, 0x18
};

static void draw_bitmap_demo(void) {
    gfx_screen(GFX_MODE_GRAPHICS2, GFX_SPRITE_8X8, 0);
    gfx_color(COLOR_WHITE, COLOR_BLACK, COLOR_DKBLUE);
    gfx_clear(GFX_COLOR_DEFAULT);
    gfx_line(8, 8, 247, 183, COLOR_LTYELLOW, GFX_LINE_NORMAL, GFX_OP_PSET);
    gfx_line(247, 8, 8, 183, COLOR_LTYELLOW, GFX_LINE_NORMAL, GFX_OP_PSET);
    gfx_line(24, 20, 232, 172, COLOR_CYAN,
             GFX_LINE_BOX | GFX_LINE_FILL, GFX_OP_PSET);
    gfx_circle(128, 96, 48, COLOR_LTRED, 0, 360, 100, GFX_OP_PSET);
    gfx_sprite_pattern(0, ball_pattern, 8);
    gfx_sprite(0, 0, COLOR_WHITE, 124, 92);
}

static void draw_tile_and_error_demo(void) {
    int color;
    static const unsigned char tile_pattern[8] = {
        0xff, 0x81, 0xbd, 0xa5, 0xa5, 0xbd, 0x81, 0xff
    };

    gfx_screen(GFX_MODE_GRAPHICS1, GFX_SPRITE_8X8, 0);
    gfx_pattern_define(1, tile_pattern, 8);
    gfx_tile(14, 10, 1, COLOR_CYAN);
    gfx_tile(15, 10, 1, COLOR_CYAN);
    gfx_tile(14, 11, 1, COLOR_CYAN);
    gfx_tile(15, 11, 1, COLOR_CYAN);
    if (gfx_point(0, 0, &color) != GFX_ERR_WRONG_MODE) {
        gfx_sprite_enable(0);
    }
}

int main(char* args) {
    struct GfxInformation info;
    (void)args;

    if (gfx_get_info(&info) != GFX_OK) return 1;
    draw_bitmap_demo();
    draw_tile_and_error_demo();
    gfx_screen(GFX_MODE_TEXT40, GFX_SPRITE_8X8, 0);
    term_puts("Graphics API demo complete.\n");
    return 0;
}
