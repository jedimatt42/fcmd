#pragma once

#define LINE_TYPE_UNKNOWN 0
#define LINE_TYPE_NORMAL 1
#define LINE_TYPE_LITERAL 2
#define LINE_TYPE_HEADING 3
#define LINE_TYPE_LINK 4
#define LINE_TYPE_QUOTE 5
#define LINE_TYPE_TOGGLE 6
#define LINE_TYPE_BULLET 7

struct __attribute__((__packed__)) Line {
  int type;
  int length;
  char data[80];
};

void init_page();
void page_clear_lines();
void page_load();
void page_from_buf(char* buf, int len);

struct Line* page_get_line(int idx);


