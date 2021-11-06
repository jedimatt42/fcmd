#include <fc_api.h>
#include <ioports.h>
#include <kscan.h>
#include "gemini.h"
#include "screen.h"
#include "page.h"
#include "link.h"
#include "mouse.h"
#include "readline.h"
#include "keyboard.h"
#include "history.h"
#include "about.h"

void send_request(char* request);
void handle_success(char* line);
void handle_default(char* line);
void on_exit();
int check_requirements();

void display_page();

char CRLF[3] = {'\r', '\n', 0};

#define SOCKET_ID 0

void halt() {
  while(1) { }
}

int fc_main(char* args) {
  if (check_requirements()) {
    return 1;
  }

  fc_strset((char*)&state, 0, sizeof(struct State));
  fc_strncpy(state.newurl, args, 256);

  init_mouse();
  init_history();
  init_page();
  init_screen();

  state.cmd = 0;
  if (state.newurl[0] == 0) {
    fc_strcpy(state.newurl, "about:");
    state.cmd = CMD_RELOAD_NOHIST;
  } else {
    state.cmd = CMD_RELOAD;
  }

  update_mouse(); // throw one away - the tipi mouse might queue a click
  while(state.cmd != CMD_QUIT) {
    int prev_cmd = state.cmd;
    int prev_lc = state.line_count;
    int prev_lo = state.line_offset;
    // handle url change here
    if (state.cmd == CMD_RELOAD || state.cmd == CMD_RELOAD_NOHIST) {
      int hist = state.cmd == CMD_RELOAD;
      open_url(state.newurl, hist);
    } else if (state.cmd == CMD_STOP) {
      state.cmd = CMD_IDLE;
    } else if (state.cmd == CMD_READPAGE) {
      page_load();
    }
    if (prev_lc != state.line_count || prev_lo != state.line_offset || prev_cmd != state.cmd) {
      screen_status();
    }
    if (prev_cmd == CMD_READPAGE && state.cmd == CMD_IDLE) {
      screen_redraw();
    }
    if (vdp_read_status()) {
      process_input();
    }
  }
  on_exit();
  return 0;
}

void process_input() {
  handle_keyboard();
  int click = update_mouse();
  if (click & MB_LEFT) {
    handle_mouse_click();
  }
}

void on_exit() {
  fc_tipi_mouse_disable();
  fc_sams_free_pages(state.page_count + 1 /* 1 for history */);
}

void open_url(char* url, int push_history) {
  state.cmd = CMD_IDLE;
  state.error[0] = 0;
  char hostname[80];
  char port[10];

  if (fc_str_startswith(url, "http:") || fc_str_startswith(url, "https:") || fc_str_startswith(url, "gopher:")) {
    return;
  }

  if (!fc_strcmp(url, "about:")) {
    about();
    state.cmd = CMD_IDLE;
    return;
  }

  update_full_url(state.url, url);
  normalize_url(state.url);
  set_hostname_and_port(state.url, hostname, port); 

  fc_strcpy(state.error, "connecting...");
  screen_status();

  if(fc_tls_connect(SOCKET_ID, hostname, port)) {
    state.error[0] = 0;
    init_readline(SOCKET_ID);
    send_request(state.url);

    char* line = readline();
    // strip any newline off
    int i = fc_indexof(line, 13);
    if (i != -1) {
      line[i] = 0;
    }
    i = fc_indexof(line, 10);
    if (i != -1) {
      line[i] = 0;
    }

    switch(line[0]) {
      case '2': 
	{
	  if (push_history) {
	    history_add_link(state.url);
	  }
	  handle_success(line);
	}
	break;
      case '1':
	{
	  char query[80];
	  fc_strset(query, 0, 80);
	  screen_prompt(query, line + 3);
	  int l = fc_strlen(state.url);
	  fc_strncpy(state.newurl, state.url, 256);
	  state.newurl[l++] = '?';
	  fc_strncpy(state.newurl + l, query, 256 - l);
	  state.cmd = CMD_RELOAD;
	}
	break;
      case '3':
	{
	  fc_strncpy(state.newurl, line + 3, 256);
	  state.cmd = CMD_RELOAD;
	}
	break;
      case '4':
	{
	  fc_strcpy(state.error, "4x error");
	}
        break;
      case '5':
	{
	  fc_strcpy(state.error, "5x error");
	}
	break;
    }
  } else {
    fc_strcpy(state.error, "Connection error");
  }
  if (state.cmd != CMD_READPAGE) {
    fc_tls_close(SOCKET_ID);
  }
}

void send_request(char* request) {
  fc_tls_send_chars(SOCKET_ID, request, fc_strlen(request));
  fc_tls_send_chars(SOCKET_ID, CRLF, 2);
}

void handle_success(char* line) {
  // gobbles the status code, we already know it is a '2x'
  char* tok = fc_strtok(line, ' ');

  tok = fc_strtok(0, ';');
  page_clear_lines(); // erase the current page
  if (fc_str_startswith(tok, "text/gemini")) {
    state.cmd = CMD_READPAGE;
  } else if (fc_str_startswith(tok, "text/plain")) {
    state.cmd = CMD_READPAGE;
  } else {
    fc_strcpy(state.error, "uknown mime-type: ");
    fc_strcpy(state.error + 18, tok);
  }
}

void any_key() {
  fc_tputs("press any key to continue.");
  while(!read_keyboard()) {
    // spin
  }
}

int check_requirements() {
  struct DisplayInformation dinfo;
  fc_display_info(&dinfo);
  int res = 0;
  if (dinfo.vdp_type != VDP_F18A) {
    fc_tputs("F18A required\n");
    res = 1;
  }
  struct SamsInformation sinfo;
  fc_sams_info(&sinfo);
  if (sinfo.total_pages == 0) {
    fc_tputs("SAMS required\n");
    res = 1;
  }
  if (res) {
    any_key();
  }
  return res;
}

