#ifndef _GEMINI_H
#define _GEMINI_H 1

struct __attribute__((__packed__)) State {
  int base_id;
  int page_id;
  int line_count;
  int line_limit;
  int line_offset;
  int page_count;
  int toggle_literal;
  int loading;
  char url[256];
  char error[80];
};

extern struct State state;

void open_url(char* url);

#endif

