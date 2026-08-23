#include "banks.h"
#define MYBANK BANK(6)

#include "graphics_internal.h"

static int parse_number(const char** cursor, int* value) {
    int sign = 1;
    int result = 0;
    int found = 0;
    while (**cursor == ' ' || **cursor == '\t' || **cursor == ',') (*cursor)++;
    if (**cursor == '+' || **cursor == '-') {
        if (**cursor == '-') sign = -1;
        (*cursor)++;
    }
    while (**cursor >= '0' && **cursor <= '9') {
        result = result * 10 + (**cursor - '0');
        (*cursor)++;
        found = 1;
    }
    if (!found) return GFX_ERR_INVALID;
    *value = result * sign;
    return GFX_OK;
}

int gfx_draw(const char* commands, int color, int op) {
    if (!commands) return GFX_ERR_INVALID;
    if (color == GFX_COLOR_DEFAULT) color = gfx_foreground;
    if (!bk_gfx_color_valid(color)) return GFX_ERR_RANGE;
    const char* cursor = commands;
    while (*cursor) {
        while (*cursor == ' ' || *cursor == '\t' || *cursor == ',') cursor++;
        if (!*cursor) break;
        char command = *cursor++;
        if (command >= 'a' && command <= 'z') command -= 'a' - 'A';
        if (command == 'M') {
            int x;
            int y;
            if (parse_number(&cursor, &x) != GFX_OK ||
                parse_number(&cursor, &y) != GFX_OK) return GFX_ERR_INVALID;
            if (bk_gfx_set_cursor(x, y) != GFX_OK) return GFX_ERR_RANGE;
            continue;
        }
        int move_only = command == 'B';
        if (move_only) {
            while (*cursor == ' ' || *cursor == '\t') cursor++;
            command = *cursor++;
            if (command >= 'a' && command <= 'z') command -= 'a' - 'A';
        }
        int distance = 1;
        if (*cursor == '+' || *cursor == '-' ||
            (*cursor >= '0' && *cursor <= '9')) {
            if (parse_number(&cursor, &distance) != GFX_OK) return GFX_ERR_INVALID;
        }
        int dx = 0;
        int dy = 0;
        switch (command) {
        case 'U': dy = -distance; break;
        case 'D': dy = distance; break;
        case 'L': dx = -distance; break;
        case 'R': dx = distance; break;
        case 'E': dx = distance; dy = -distance; break;
        case 'F': dx = distance; dy = distance; break;
        case 'G': dx = -distance; dy = distance; break;
        case 'H': dx = -distance; dy = -distance; break;
        default: return GFX_ERR_INVALID;
        }
        int next_x = gfx_cursor_x + dx;
        int next_y = gfx_cursor_y + dy;
        if (move_only) {
            if (bk_gfx_set_cursor(next_x, next_y) != GFX_OK) return GFX_ERR_RANGE;
        } else {
            int result = bk_gfx_line(gfx_cursor_x, gfx_cursor_y, next_x,
                                     next_y, color, GFX_LINE_NORMAL, op);
            if (result != GFX_OK) return result;
            gfx_cursor_x = next_x;
            gfx_cursor_y = next_y;
        }
    }
    return GFX_OK;
}

int gfx_copy(int source_page, int destination_page, int x1, int y1,
             int x2, int y2, int destination_x, int destination_y, int op) {
    if (!(gfx_info.capabilities & GFX_CAP_COPY)) return GFX_ERR_UNSUPPORTED;
    if (source_page < 0 || destination_page < 0 ||
        source_page >= gfx_info.page_count ||
        destination_page >= gfx_info.page_count) return GFX_ERR_RANGE;
    if (x1 < 0 || y1 < 0 || x2 < x1 || y2 < y1 ||
        x2 >= gfx_info.width || y2 >= gfx_info.height) return GFX_ERR_RANGE;
    if (destination_x < 0 || destination_y < 0 ||
        destination_x + x2 - x1 >= gfx_info.width ||
        destination_y + y2 - y1 >= gfx_info.height) return GFX_ERR_RANGE;
    int width = x2 - x1 + 1;
    int height = y2 - y1 + 1;
    unsigned int source_base = (unsigned int)source_page * 0x8000;
    unsigned int destination_base = (unsigned int)destination_page * 0x8000;
    if (source_page != destination_page) {
        int colors[512];
        int row;
        for (row = 0; row < height; row++) {
            int col;
            for (col = 0; col < width; col++) {
                int result = bk_gfx_get_pixel_base(source_base, x1 + col,
                                                   y1 + row, &colors[col]);
                if (result != GFX_OK) return result;
            }
            for (col = 0; col < width; col++) {
                int result = bk_gfx_set_pixel_base(destination_base,
                                                   destination_x + col,
                                                   destination_y + row,
                                                   colors[col], op);
                if (result != GFX_OK) return result;
            }
        }
        return GFX_OK;
    }
    int y_start = 0;
    int y_end = height;
    int y_step = 1;
    int x_start = 0;
    int x_end = width;
    int x_step = 1;
    if (destination_y > y1) { y_start = height - 1; y_end = -1; y_step = -1; }
    if (destination_x > x1) { x_start = width - 1; x_end = -1; x_step = -1; }
    int row;
    for (row = y_start; row != y_end; row += y_step) {
        int col;
        for (col = x_start; col != x_end; col += x_step) {
            int color;
            int result = bk_gfx_get_pixel_base(source_base, x1 + col,
                                               y1 + row, &color);
            if (result != GFX_OK) return result;
            result = bk_gfx_set_pixel_base(destination_base,
                                           destination_x + col,
                                           destination_y + row, color, op);
            if (result != GFX_OK) return result;
        }
    }
    return GFX_OK;
}
