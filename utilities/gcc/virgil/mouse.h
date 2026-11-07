#pragma once

#include <fc_api.h>

extern int mouse_active;

extern struct MouseData md;

FC_SAMS_VOIDBANKED(0, init_mouse, (), ());

FC_SAMS_BANKED(0, int, update_mouse, (), ());

FC_SAMS_VOIDBANKED(0, handle_mouse_click, (), ());



