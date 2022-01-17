#pragma once

#include <fc_api.h>
#include "page.h"

struct __attribute__((__packed__)) Link {
  char data[80];
};

FC_SAMS_VOIDBANKED(1, init_history, (), ());
FC_SAMS_VOIDBANKED(1, history_add_link, (char* link), (link));
FC_SAMS_VOIDBANKED(1, history_get_prev, (char* dst), (dst));
FC_SAMS_BANKED(1, int, history_get, (char* dst, int i), (dst, i));
FC_SAMS_VOIDBANKED(1, show_history, (), ());

