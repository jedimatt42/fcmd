#ifndef _GEMINI_H
#define _GEMINI_H 1

struct __attribute__((__packed__)) State {
  int history_id;
  int base_id;
  int page_id;
  int line_count;
  int line_limit;
  int line_offset;
  int page_count;
  int toggle_literal;
  int loading;
  int cmd;
  char url[256];
  char newurl[256];
  char error[80];
};

#define CMD_QUIT 1
#define CMD_STOP 2
#define CMD_RELOAD 3
#define CMD_RELOAD_NOHIST 4

extern struct State state;

void open_url(char* url, int push_history);

#endif

