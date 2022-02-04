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
#include "bookmarks.h"
#include "fileload.h"
#include "about.h"

void send_request(char* request);
void handle_success(char* line);
void handle_default(char* line);
void on_exit();
void restore_url();
int check_requirements();

void display_page();
int must_redraw();

const char CRLF[3] = {'\r', '\n', 0};

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

  init_history();
  init_page();
  init_mouse();
  init_screen();

  state.cmd = CMD_IDLE;
  if (state.newurl[0] == 0) {
    fc_strcpy(state.newurl, "about:");
    state.cmd = CMD_RELOAD;
  } else {
    state.cmd = CMD_RELOAD;
  }

  update_mouse(); // throw one away - the tipi mouse might queue a click
  while(state.cmd != CMD_QUIT) {
    int prev_cmd = state.cmd;
    int prev_lc = state.line_count;
    int prev_lo = state.line_offset;

    // handle url change here
    if (state.cmd == CMD_RELOAD) {
      open_url(state.newurl);
    } else if (state.cmd == CMD_STOP) {
      state.cmd = CMD_IDLE;
      screen_redraw();
    } else if (state.cmd == CMD_READPAGE) {
      if (mouse_active) {
	mouse_active--;
      } else {
	page_load();
      }
    }
    if (must_redraw(prev_lc, prev_lo, prev_cmd, &state)) {
      screen_redraw();
    }
    if (prev_lc != state.line_count || prev_lo != state.line_offset || prev_cmd != state.cmd) {
      screen_status();
    }
    if (vdp_read_status()) {
      if (state.error_ticks) {
        state.error_ticks--;
	if (state.error_ticks == 0) {
          screen_status();
	}
      }
      process_input();
    }
  }
  on_exit();
  return 0;
}

int must_redraw(int prev_lc, int prev_lo, int prev_cmd, struct State* state) {
  // redraw if we transitioned to IDLE
  if (prev_cmd != CMD_IDLE && state->cmd == CMD_IDLE) {
    return 1;
  }
  // redraw if there have been more lines, our screen isn't full, and we are not IDLE
  if (state->cmd != CMD_IDLE) {
    if (state->line_count >= prev_lc && (state->line_offset + 28 <= state->line_count)) {
      return 1;
    }
  }
  return 0;
}

void FC_SAMS(0, process_input()) {
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

void prepare_for_builtin_url(char* url) {
    page_clear_lines();
    state.cmd = CMD_IDLE;
    fc_strcpy(state.url, url);
    history_add_link(state.url);
    state.history_pop = 1;
}

void FC_SAMS(0, open_url(char* url)) {
  state.cmd = CMD_IDLE;
  state.error[0] = 0;
  char hostname[80];
  char port[10];

  state.history_pop = 0;

  if (fc_str_startswith(url, "http:") || fc_str_startswith(url, "https:") || fc_str_startswith(url, "gopher:")) {
    set_error("unsupported protocol", 0);
    return;
  }

  if (!fc_strcmp(url, "about:")) {
    prepare_for_builtin_url(url);
    about();
    return;
  }

  if (!fc_strcmp(url, "history:")) {
    prepare_for_builtin_url(url);
    show_history();
    return;
  }

  if (!fc_strcmp(url, "bookmarks:")) {
    prepare_for_builtin_url(url);
    show_bookmarks();
    return;
  }

  if (file_exists(url)) {
    page_clear_lines();
    file_load(url);
    return;
  }

  update_full_url(state.url, url);
  normalize_url(state.url);
  set_hostname_and_port(state.url, hostname, port); 

  set_error("connecting...", 0x7fff);
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
	  history_add_link(state.url);
	  state.history_pop = 1;
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
	  set_error("4x error", 0);
	}
        break;
      case '5':
	{
	  set_error("5x error", 0);
	  restore_url();
	}
	break;
    }
  } else {
    set_error("Connection error", 0);
    restore_url();
  }
  if (state.cmd != CMD_READPAGE) {
    fc_tls_close(SOCKET_ID);
  }
}

void restore_url() {
  char tmp[256];
  history_get_prev(tmp);
  update_full_url(state.url, tmp);
}

void send_request(char* request) {
  char tmp[256];
  fc_strcpy(tmp, request);
  fc_strcat(tmp, CRLF);
  fc_tls_send_chars(SOCKET_ID, tmp, fc_strlen(tmp));
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
    char msg[80];
    fc_strcpy(msg, "uknown mime-type: ");
    fc_strncpy(msg + 18, tok, 80-18);
    set_error(msg, 0);
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

void FC_SAMS(0, set_error(char* msg, int ticks)) {
  fc_strncpy(state.error, msg, 80);
  if (ticks == 0) {
    ticks = 15*60; // 60 ticks a second
  }
  state.error_ticks = ticks;
}
