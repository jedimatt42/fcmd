#pragma once

#include <fc_api.h>

#define _PAGE_PAGE 1

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

FC_SAMS_VOIDBANKED(_PAGE_PAGE, init_page, (), ());
FC_SAMS_VOIDBANKED(_PAGE_PAGE, page_clear_lines, (), ());
FC_SAMS_VOIDBANKED(_PAGE_PAGE, page_load, (), ());
FC_SAMS_VOIDBANKED(_PAGE_PAGE, page_from_buf, (char* buf, int len), (buf, len));

FC_SAMS_BANKED(_PAGE_PAGE, struct Line*, page_get_line, (int idx), (idx));


