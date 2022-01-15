#pragma once

#include <fc_api.h>

#define LINE_TYPE_UNKNOWN 0
#define LINE_TYPE_NORMAL 1
#define LINE_TYPE_LITERAL 2
#define LINE_TYPE_HEADING 3
#define LINE_TYPE_LINK 4
#define LINE_TYPE_QUOTE 5
#define LINE_TYPE_TOGGLE 6
#define LINE_TYPE_BULLET 7
#define LINE_TYPE_LINK_CONT 8

struct __attribute__((__packed__)) Line {
  int type;
  int length;
  char data[80];
};

FC_SAMS_VOIDBANKED(0, init_page, (), ());
FC_SAMS_VOIDBANKED(0, page_clear_lines, (), ());
FC_SAMS_VOIDBANKED(0, page_load, (), ());
FC_SAMS_VOIDBANKED(0, page_from_buf, (char* buf, int len), (buf, len));

FC_SAMS_BANKED(0, struct Line*, page_get_line, (int idx), (idx));


