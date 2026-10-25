#include "banks.h"
#define MYBANK BANK(11)

#include "tui_internal.h"
#include "globals.h"
#include "vdp.h"

struct tui_state* tui_ctx;

int tui_init(void* pool, int size) {
    if (size < (int)sizeof(struct tui_state)) {
        return -1;
    }
    tui_ctx = (struct tui_state*)pool;
    tui_ctx->windows = 0;
    tui_ctx->focus = 0;
    tui_ctx->widget_under_mouse = 0;
    tui_ctx->mouse.mx = 0;
    tui_ctx->mouse.my = 0;
    tui_ctx->mouse.buttons = 0;
    tui_ctx->mouse_on = 0;
    tui_ctx->mouse_depth = 0;
    tui_ctx->mouse_prev = 0;
    tui_ctx->mouse_col = 0;
    tui_ctx->mouse_row = 0;
    tui_ctx->pool_start = (unsigned char*)pool + sizeof(struct tui_state);
    tui_ctx->pool_end = (unsigned char*)pool + size;
    tui_ctx->pool_cur = tui_ctx->pool_start;
    return 0;
}

void tui_done() {
    if (tui_ctx && tui_ctx->mouse_depth) {
        tui_ctx->mouse_depth = 1;
        tui_mouse_disable();
    }
    tui_ctx = 0;
}

int tui_screen_width() {
    return displayWidth;
}

int tui_screen_height() {
    return displayHeight;
}

int tui_has_color() {
    return (nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES) ? 1 : 0;
}
