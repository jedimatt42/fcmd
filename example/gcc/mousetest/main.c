#include "fctest.h"

/*
 * I19 - TIPI mouse.
 *
 * Covers: mouse_read, mouse_show, mouse_move, mouse_set_pointer, mouse_hide.
 *
 * Mouse support exists only on F18A in 80x30 mode; otherwise the whole test is
 * skipped. mouse_read talks to the TIPI; without a TIPI/mouse the reads are
 * no-ops, so the movement checks still hold.
 */

/* MP_* constants are internal to tipi_mouse.h and not part of the SDK */
#define MOUSE_PTR_BUSY 6

static int bounds_ok;
static unsigned int seen_buttons;

static void test_mouse(void) {
    struct GfxInformation info;
    struct MouseData md;
    struct MouseData rd;
    unsigned int last_buttons;

    if (gfx_get_info(&info) != GFX_OK) {
        FC_SKIP("no display information");
        return;
    }
    if (!(info.vdp_type == VDP_F18A && info.height == 30)) {
        FC_SKIP("mouse requires F18A 80x30");
        return;
    }

    /* mouse_read fills only the first three bytes of the structure */
    str_set((char*)&rd, 0, sizeof(rd));
    mouse_read(&rd);
    FC_CHECK(((unsigned char)rd.buttons & 0xF8) == 0);

    str_set((char*)&md, 0, sizeof(md));
    mouse_show(&md);
    FC_CHECK_EQ(md.pointerx, 128);
    FC_CHECK_EQ(md.pointery, 96);

    term_puts("[OBSERVE] a pointer should appear in the middle of the screen\n");
    term_puts("move the mouse and click the buttons; press a key to stop\n");

    bounds_ok = 1;
    seen_buttons = 0;
    last_buttons = 0;
    while ((KSCAN_STATUS & KSCAN_MASK) == 0) {
        term_kscan(5);   /* the keyboard must be scanned for the status bit to update */
        mouse_move(&md);
        if (md.pointerx < 8 || md.pointerx > 247) {
            bounds_ok = 0;
        }
        if (md.pointery < 0 || md.pointery > 238) {
            bounds_ok = 0;
        }
        if (md.buttons != last_buttons) {
            term_puts("[OBSERVE] buttons = ");
            term_puts(str_from_uint((unsigned int)md.buttons));
            if (md.buttons & MB_LEFT) {
                term_puts(" LEFT");
            }
            if (md.buttons & MB_RIGHT) {
                term_puts(" RIGHT");
            }
            if (md.buttons & MB_MID) {
                term_puts(" MID");
            }
            term_putc('\n');
            last_buttons = (unsigned char)md.buttons;
        }
        if (md.buttons) {
            seen_buttons |= (unsigned int)md.buttons;
        }
    }
    term_kscan(5);
    FC_CHECK(bounds_ok);

    /* the public mask must never contain bits outside MB_* */
    FC_CHECK((seen_buttons & ~(unsigned int)(MB_LEFT | MB_RIGHT | MB_MID)) == 0);

    term_puts("final pointer: ");
    term_puts(str_from_uint((unsigned int)md.pointerx));
    term_putc(',');
    term_puts(str_from_uint((unsigned int)md.pointery));
    term_putc('\n');

    term_puts("[OBSERVE] buttons observed:");
    if (seen_buttons & MB_LEFT) {
        term_puts(" LEFT");
    }
    if (seen_buttons & MB_RIGHT) {
        term_puts(" RIGHT");
    }
    if (seen_buttons & MB_MID) {
        term_puts(" MID");
    }
    if (seen_buttons == 0) {
        term_puts(" (none)");
    }
    term_putc('\n');

    mouse_set_pointer(MOUSE_PTR_BUSY);
    FC_OBSERVE("the pointer should change to the busy style");
    FC_WAIT("press a key to continue");
    mouse_set_pointer(0);

    mouse_hide();
    FC_OBSERVE("the pointer should now be hidden");
}

int main(char* args) {
    (void)args;

    term_puts("MOUSETEST test\n");

    test_mouse();

    return fc_summary();
}
