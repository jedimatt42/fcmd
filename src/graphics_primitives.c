#include "banks.h"
#define MYBANK BANK(13)

#include "graphics_internal.h"

int gfx_line(int x1, int y1, int x2, int y2, int color, int style, int op) {
    if (!(gfx_info.capabilities & GFX_CAP_LINES)) return GFX_ERR_WRONG_MODE;
    if (style & ~(GFX_LINE_BOX | GFX_LINE_FILL)) return GFX_ERR_INVALID;
    if (x1 < 0 || x1 >= gfx_info.width || x2 < 0 || x2 >= gfx_info.width ||
        y1 < 0 || y1 >= gfx_info.height || y2 < 0 || y2 >= gfx_info.height) {
        return GFX_ERR_RANGE;
    }
    if (color == GFX_COLOR_DEFAULT) color = gfx_foreground;
    if (!bk_gfx_color_valid(color)) return GFX_ERR_RANGE;
    if (style & GFX_LINE_FILL) {
        int left = x1 < x2 ? x1 : x2;
        int right = x1 > x2 ? x1 : x2;
        int top = y1 < y2 ? y1 : y2;
        int bottom = y1 > y2 ? y1 : y2;
        int x;
        int y;
        for (y = top; y <= bottom; y++) {
            for (x = left; x <= right; x++) {
                int result = bk_gfx_pset(x, y, color, op);
                if (result != GFX_OK) return result;
            }
        }
        if (!(style & GFX_LINE_BOX)) return GFX_OK;
    }
    if (style & GFX_LINE_BOX) {
        int result = gfx_line(x1, y1, x2, y1, color, GFX_LINE_NORMAL, op);
        if (result != GFX_OK) return result;
        result = gfx_line(x2, y1, x2, y2, color, GFX_LINE_NORMAL, op);
        if (result != GFX_OK) return result;
        result = gfx_line(x2, y2, x1, y2, color, GFX_LINE_NORMAL, op);
        if (result != GFX_OK) return result;
        return gfx_line(x1, y2, x1, y1, color, GFX_LINE_NORMAL, op);
    }
    int dx = x2 > x1 ? x2 - x1 : x1 - x2;
    int sx = x1 < x2 ? 1 : -1;
    int dy = y2 > y1 ? y2 - y1 : y1 - y2;
    int sy = y1 < y2 ? 1 : -1;
    int error = (dx > dy ? dx : -dy) / 2;
    while (1) {
        int result = bk_gfx_pset(x1, y1, color, op);
        if (result != GFX_OK) return result;
        if (x1 == x2 && y1 == y2) break;
        int e2 = error;
        if (e2 > -dx) { error -= dy; x1 += sx; }
        if (e2 < dy) { error += dx; y1 += sy; }
    }
    return GFX_OK;
}

int gfx_circle(int center_x, int center_y, int radius, int color,
               int start_angle, int end_angle, int aspect, int op) {
    if (!(gfx_info.capabilities & GFX_CAP_CIRCLES)) return GFX_ERR_WRONG_MODE;
    if (radius < 0 || aspect <= 0 || start_angle < 0 || end_angle < 0 ||
        start_angle > 360 || end_angle > 360) return GFX_ERR_INVALID;
    if (start_angle != 0 || end_angle != 360) return GFX_ERR_UNSUPPORTED;
    if (color == GFX_COLOR_DEFAULT) color = gfx_foreground;
    if (!bk_gfx_color_valid(color)) return GFX_ERR_RANGE;
    if (center_x - radius < 0 || center_x + radius >= gfx_info.width ||
        center_y - (radius * aspect) / 100 < 0 ||
        center_y + (radius * aspect) / 100 >= gfx_info.height) {
        return GFX_ERR_RANGE;
    }
    int x = radius;
    int y = 0;
    int error = 1 - radius;
    while (x >= y) {
        int ys = (y * aspect) / 100;
        int xs = (x * aspect) / 100;
        int result;
        result = bk_gfx_pset(center_x + x, center_y + ys, color, op); if (result) return result;
        result = bk_gfx_pset(center_x + y, center_y + xs, color, op); if (result) return result;
        result = bk_gfx_pset(center_x - y, center_y + xs, color, op); if (result) return result;
        result = bk_gfx_pset(center_x - x, center_y + ys, color, op); if (result) return result;
        result = bk_gfx_pset(center_x - x, center_y - ys, color, op); if (result) return result;
        result = bk_gfx_pset(center_x - y, center_y - xs, color, op); if (result) return result;
        result = bk_gfx_pset(center_x + y, center_y - xs, color, op); if (result) return result;
        result = bk_gfx_pset(center_x + x, center_y - ys, color, op); if (result) return result;
        y++;
        if (error <= 0) error += 2 * y + 1;
        else { x--; error += 2 * (y - x) + 1; }
    }
    return GFX_OK;
}

struct PaintPoint {
    int x;
    int y;
};

static struct PaintPoint paint_stack[256];

int gfx_paint(int x, int y, int color, int border_color, int op) {
    if (!(gfx_info.capabilities & GFX_CAP_PAINT)) return GFX_ERR_WRONG_MODE;
    if (color == GFX_COLOR_DEFAULT) color = gfx_foreground;
    if (border_color == GFX_COLOR_DEFAULT) border_color = gfx_foreground;
    if (!bk_gfx_color_valid(color) || !bk_gfx_color_valid(border_color)) {
        return GFX_ERR_RANGE;
    }
    int target;
    int result = bk_gfx_point(x, y, &target);
    if (result != GFX_OK) return result;
    if (target == border_color || target == color) return GFX_OK;
    int top = 0;
    paint_stack[top++] = (struct PaintPoint){x, y};
    while (top) {
        struct PaintPoint point = paint_stack[--top];
        int current;
        if (point.x < 0 || point.y < 0 || point.x >= gfx_info.width ||
            point.y >= gfx_info.height) continue;
        if (bk_gfx_point(point.x, point.y, &current) != GFX_OK ||
            current != target) continue;
        result = bk_gfx_pset(point.x, point.y, color, op);
        if (result != GFX_OK) return result;
        if (top + 4 > 256) return GFX_ERR_BUSY;
        paint_stack[top++] = (struct PaintPoint){point.x + 1, point.y};
        paint_stack[top++] = (struct PaintPoint){point.x - 1, point.y};
        paint_stack[top++] = (struct PaintPoint){point.x, point.y + 1};
        paint_stack[top++] = (struct PaintPoint){point.x, point.y - 1};
    }
    return GFX_OK;
}
