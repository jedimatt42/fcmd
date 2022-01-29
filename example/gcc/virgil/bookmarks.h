#pragma once

#include <fc_api.h>
#include "page.h"

struct __attribute__((__packed__)) Bookmark {
  char data[80];
};

FC_SAMS_VOIDBANKED(0, init_bookmarks, (), ());
FC_SAMS_VOIDBANKED(0, bookmarks_add_link, (char* link), (link));
FC_SAMS_VOIDBANKED(0, bookmarks_get_prev, (char* dst), (dst));
FC_SAMS_BANKED(0, int, bookmarks_get, (char* dst, int i), (dst, i));
FC_SAMS_VOIDBANKED(0, show_bookmarks, (), ());

