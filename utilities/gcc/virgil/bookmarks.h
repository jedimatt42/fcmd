#pragma once

#include <fc_api.h>

FC_SAMS_BANKED(2, int, bookmarks_add_link, (char* link), (link));
FC_SAMS_VOIDBANKED(2, show_bookmarks, (), ());

