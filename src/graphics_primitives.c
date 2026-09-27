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

/*
 * Shared midpoint-circle algorithm with the vertical component scaled to
 * radius_y / radius_x, which draws an ellipse. A circle is the radius_x ==
 * radius_y case. Only full (0..360) shapes are supported.
 */
static int gfx_ellipse_raw(int center_x, int center_y, int radius_x,
                           int radius_y, int color, int op) {
    if (!(gfx_info.capabilities & GFX_CAP_CIRCLES)) return GFX_ERR_WRONG_MODE;
    if (radius_x <= 0 || radius_y <= 0) return GFX_ERR_INVALID;
    if (color == GFX_COLOR_DEFAULT) color = gfx_foreground;
    if (!bk_gfx_color_valid(color)) return GFX_ERR_RANGE;
    if (center_x - radius_x < 0 || center_x + radius_x >= gfx_info.width ||
        center_y - radius_y < 0 || center_y + radius_y >= gfx_info.height) {
        return GFX_ERR_RANGE;
    }
    {
        int x = radius_x;
        int y = 0;
        int error = 1 - radius_x;
        /* Unsigned division: signed variable division is miscompiled here. */
        unsigned int k100 = ((unsigned int)radius_y * 100u) / (unsigned int)radius_x;
        while (x >= y) {
            int ys = (int)(((unsigned int)y * k100) / 100u);
            int xs = (int)(((unsigned int)x * k100) / 100u);
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
            if (error <= 0) {
                error += 2 * y + 1;
            } else {
                x--;
                error += 2 * (y - x) + 1;
            }
        }
    }
    return GFX_OK;
}

int gfx_circle(int center_x, int center_y, int radius, int color, int op) {
    /* The 512-wide modes (GRAPHICS5/GRAPHICS6) use half-width pixels, so the
       horizontal pixel radius is doubled to keep the shape round on screen. */
    int radius_x = (gfx_info.width == 512) ? radius * 2 : radius;
    return gfx_ellipse_raw(center_x, center_y, radius_x, radius, color, op);
}

int gfx_ellipse(int center_x, int center_y, int radius_x, int radius_y,
                int color, int op) {
    return gfx_ellipse_raw(center_x, center_y, radius_x, radius_y, color, op);
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
