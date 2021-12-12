#pragma once

#include <fc_api.h>

FC_SAMS_VOIDBANKED(0, handle_keyboard, (), ());
FC_SAMS_BANKED(0, int, read_keyboard, (), ());
FC_SAMS_BANKED(0, int, on_key_up, (), ());
FC_SAMS_BANKED(0, int, on_key_down, (), ());
FC_SAMS_BANKED(0, int, on_page_up, (), ());
FC_SAMS_BANKED(0, int, on_page_down, (), ());
FC_SAMS_VOIDBANKED(0, on_back, (), ());
FC_SAMS_VOIDBANKED(0, on_address, (), ());

