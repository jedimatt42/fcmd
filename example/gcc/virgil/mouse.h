#pragma once

#include <fc_api.h>

#define MP_NORMAL 0
#define MP_WAIT 1
#define MP_SCROLL_DOWN 2
#define MP_SCROLL_UP 3
#define MP_PAGE_DOWN 4
#define MP_PAGE_UP 5
#define MP_BUSY 6

extern int mouse_active;

extern struct MouseData md;

FC_SAMS_VOIDBANKED(0, init_mouse, (), ());

FC_SAMS_BANKED(0, int, update_mouse, (), ());

FC_SAMS_VOIDBANKED(0, handle_mouse_click, (), ());

FC_SAMS_VOIDBANKED(0, mouse_set_pointer, (int p), (p));

