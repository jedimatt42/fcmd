#include "banks.h"
#define MYBANK BANK(11)

#include "tui_internal.h"
#include "terminal.h"
#include "tipi_mouse.h"
#include "editor.h"

int tui_get_event(struct tui_event* ev) {
    if (!ev) return 0;
    ev->type = TUI_EV_NONE;
    ev->widget = 0;
    ev->key = 0;
    ev->mx = 0;
    ev->my = 0;
    ev->buttons = 0;

    int k = (int)bk_kscan(5);
    if (k != 255) {
        ev->key = k;
        ev->type = TUI_EV_KEY;

        if (k == 6) {
            tui_focus_next();
            return 1;
        }
        if (k == 2) {
            if (tui_ctx->windows) {
                tui_win_t* top = tui_ctx->windows;
                tui_win_t* last = top;
                while (last->next) last = last->next;
                if (last->children == 0 || tui_ctx->focus == 0) {
                    tui_win_close(last);
                    return 1;
                }
            }
        }
        if (tui_ctx->focus) {
            int handled = tui_handle_widget_key(tui_ctx->focus, k);
            if (handled) {
                ev->type = TUI_EV_WIDGET_ACTIVATED;
                ev->widget = tui_ctx->focus;
                return 1;
            }
        }
        return 1;
    }

    return 0;
}

int tui_dispatch_event(struct tui_event* ev) {
    if (!ev || ev->type == TUI_EV_NONE) return 0;
    return 1;
}
