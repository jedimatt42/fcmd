#pragma once

#include <fc_api.h>
#include "page.h"

struct Link {
  char data[80];
};

FC_SAMS_VOIDBANKED(0, init_history, (), ());
FC_SAMS_VOIDBANKED(0, history_push, (char* link), (link));
FC_SAMS_VOIDBANKED(0, history_pop, (char* dst), (dst));
FC_SAMS_BANKED(0, int, history_get, (char* dst, int i), (dst, i));
FC_SAMS_VOIDBANKED(0, show_history, (), ());

