#pragma once

#include <fc_api.h>
#include "page.h"

#define _HISTORY_BANK 0

struct __attribute__((__packed__)) Link {
  char data[80];
};

FC_SAMS_VOIDBANKED(_HISTORY_BANK, init_history, (), ());
FC_SAMS_VOIDBANKED(_HISTORY_BANK, history_add_link, (char* link), (link));
FC_SAMS_VOIDBANKED(_HISTORY_BANK, history_get_prev, (char* dst), (dst));
FC_SAMS_BANKED(_HISTORY_BANK, int, history_get, (char* dst, int i), (dst, i));
FC_SAMS_VOIDBANKED(_HISTORY_BANK, show_history, (), ());

