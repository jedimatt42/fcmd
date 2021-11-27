#pragma once

struct __attribute__((__packed__)) State {
  volatile int history_id;
  volatile int base_id;
  volatile int line_count;
  volatile int line_limit;
  volatile int line_offset;
  volatile int page_count;
  volatile int toggle_literal;
  volatile int cmd;
  volatile int utfstate;
  int max_page;
  char url[256];
  char newurl[256];
  char error[80];
};

#define CMD_IDLE 0
#define CMD_QUIT 1
#define CMD_STOP 2
#define CMD_RELOAD 3
#define CMD_RELOAD_NOHIST 4
#define CMD_READPAGE 5

extern struct State state;

void open_url(char* url, int push_history);
void process_input();

inline int vdp_read_status() {
  int status;
  __asm__( "movb @>8802,%0" : "=rm" (status) : : "r12" );
  return status;
}

