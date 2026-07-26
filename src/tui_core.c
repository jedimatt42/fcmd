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
    tui_ctx->pool_start = (unsigned char*)pool + sizeof(struct tui_state);
    tui_ctx->pool_end = (unsigned char*)pool + size;
    tui_ctx->pool_cur = tui_ctx->pool_start;
    return 0;
}

void tui_done() {
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
