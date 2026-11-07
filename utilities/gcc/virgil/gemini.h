#pragma once

#include <fc_api.h>
#include "about.h"

struct State {
  volatile int history_id;
  volatile int bookmarks_id;
  volatile int base_id;
  volatile int line_count;
  volatile int line_limit;
  volatile int line_offset;
  volatile int page_count;
  volatile int toggle_literal;
  volatile int cmd;
  volatile int utfstate;
  volatile int menu_open;
  volatile int builtin;
  volatile int error_ticks;
  volatile int history_count;
  int max_page;
  char lasturl[256];
  char newurl[256];
  char error[80];
};

#define MAX_URL_LEN 255

#define CMD_IDLE 0
#define CMD_QUIT 1
#define CMD_STOP 2
#define CMD_RELOAD 3
#define CMD_READPAGE 5
#define CMD_DOWNLOAD 6

extern struct State state;

FC_SAMS_VOIDBANKED(0, open_url, (char* url), (url));
FC_SAMS_VOIDBANKED(0, process_input, (), ());
FC_SAMS_VOIDBANKED(0, set_error, (char* msg, int ticks), (msg, ticks));

inline int vdp_read_status() {
  int status;
  __asm__( "movb @>8802,%0" : "=rm" (status) );
  return status;
}


