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

void process_input();
void send_request(char* request);
void handle_success(char* line);
void handle_default(char* line);
void on_exit();
int check_requirements();

void display_page();

char CRLF[3] = {'\r', '\n', 0};

#define SOCKET_ID 0

#define VDP_WAIT_VBLANK_CRU	  __asm__( "clr r12\n\ttb 2\n\tjeq -4\n\tmovb @>8802,r12" : : : "r12" );

void halt() {
  while(1) { }
}

int fc_main(char* args) {
  if (check_requirements()) {
    return 1;
  }

  fc_strset((char*)&state, 0, sizeof(struct State));
  fc_strncpy(state.url, args, 256);

  init_mouse();
  init_history();
  init_page();
  init_screen();

  if (state.url[0] != 0) {
    open_url(state.url, 1);
  } else {
    screen_prompt(state.url, "Address:");
    if (state.url[0] != 0) {
      open_url(state.url, 1);
    }
  }

  update_mouse(); // throw one away - the tipi mouse might queue a click
  state.quit = 0;
  while(!state.quit) {
    // reset state flags
    state.stop = 0;
    // handle url change here
    if (state.reload) {
      int hist = state.reload == RELOAD;
      state.reload = 0;
      open_url(state.url, hist);
    }
    VDP_WAIT_VBLANK_CRU
    process_input();
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
  state.loading = 1;
  state.stop = 0;
  state.error[0] = 0;
  char hostname[80];
  char port[10];

  screen_status();

  update_full_url(state.url, url);
  set_hostname_and_port(state.url, hostname, port); 

  int err = fc_tls_connect(SOCKET_ID, hostname, port);
  if (err /* 0 indicates failure */) {
    err = 0;
    init_readline(SOCKET_ID);
    send_request(state.url);

    char* line = readline();
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
	  screen_prompt(query, line);
	  int l = fc_strlen(state.url);
	  state.url[l++] = '?';
	  fc_strcpy(state.url + l, query);
	  state.reload = RELOAD;
	}
	break;
      default:
	handle_default(line);
	break;
    }
    fc_tls_close(SOCKET_ID);
  } else {
    fc_strcpy(state.error, "Connection error");
  }
  fc_tls_close(SOCKET_ID);
  state.loading = 0;
  state.stop = 0;
  screen_status();
}

void send_request(char* request) {
  fc_tls_send_chars(SOCKET_ID, request, fc_strlen(request));
  fc_tls_send_chars(SOCKET_ID, CRLF, 2);
}

void handle_default(char* line) {
  fc_strcpy(state.error, line);
  screen_status();
}

void handle_success(char* line) {
  // gobbles the status code, we already know it is a '2x'
  char* tok = fc_strtok(line, ' ');

  tok = fc_strtok(0, ';');
  if (fc_str_startswith(tok, "text/gemini")) {
    display_page();
  } else if (fc_str_startswith(tok, "text/plain")) {
    display_page();
  } else {
    fc_tputs("uknown mime-type: ");
    fc_tputs(tok);
  }
}

void display_page() {
  state.line_offset = 0;
  page_clear_lines(); // erase the current page

  char* line = readline();
  while(line && !state.stop) {
    page_add_line(line);
    if (state.line_count <= 28) {
      screen_redraw();
    } else {
      screen_status();
    }
    process_input(); 
    line = readline();
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

