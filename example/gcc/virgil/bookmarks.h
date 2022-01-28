#pragma once

#include <fc_api.h>
#include "page.h"

#define _BOOKMARKS_BANK 0

struct __attribute__((__packed__)) Bookmark {
  char data[80];
};

FC_SAMS_VOIDBANKED(_BOOKMARKS_BANK, init_bookmarks, (), ());
FC_SAMS_VOIDBANKED(_BOOKMARKS_BANK, bookmarks_add_link, (char* link), (link));
FC_SAMS_VOIDBANKED(_BOOKMARKS_BANK, bookmarks_get_prev, (char* dst), (dst));
FC_SAMS_BANKED(_BOOKMARKS_BANK, int, bookmarks_get, (char* dst, int i), (dst, i));
FC_SAMS_VOIDBANKED(_BOOKMARKS_BANK, show_bookmarks, (), ());

