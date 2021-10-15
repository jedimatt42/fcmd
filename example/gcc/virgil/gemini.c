#include <fc_api.h>
#include <ioports.h>
#include <kscan.h>
#include "gemini.h"
#include "screen.h"
#include "page.h"
#include "mouse.h"
#include "readline.h"
#include "keyboard.h"

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
  init_page();
  init_screen();

  if (state.url[0] != 0) {
    open_url(state.url);
  }

  update_mouse(); // throw one away - the tipi mouse might queue a click
  while(1) {
    int click = update_mouse();
    VDP_WAIT_VBLANK_CRU
    if (click & MB_LEFT) {
      on_exit(); // Temporary
      return 0;
    } else {
      if (handle_keyboard()) {
        on_exit();
	return 0;
      }
    }
  }
}

void on_exit() {
  fc_tipi_mouse_disable();
  fc_sams_free_pages(state.page_count);
}

void set_hostname_and_port(char* url, char* hostname, char* port) {
  hostname[0] = 0;
  fc_strcpy(port, "1965");
  if (fc_str_startswith(url, "gemini://")) {
    int h = fc_indexof(url + 9, '/');
    int p = fc_indexof(url + 9, ':');
    if (h == -1) {
      h = fc_strlen(url + 9);
    }
    if (p != -1) {
      fc_strncpy(port, url + 9 + 1 + p, h - p + 1);
      h -= h - p;
    }
    fc_strncpy(hostname, url + 9, h);
  } else {
    // maybe a different schema - which we'll error on - or be relative to
    // our previous url
  }
}

void open_url(char* url) {
  char hostname[80];
  char port[10];

  set_hostname_and_port(url, hostname, port); 

  int err = fc_tls_connect(SOCKET_ID, hostname, port);
  if (err /* 0 indicates failure */) {
    err = 0;
    init_readline(SOCKET_ID);
    send_request(url);

    char* line = readline();
    switch(line[0]) {
      case '2':
	handle_success(line);
	break;
      default:
	handle_default(line);
	break;
    }
    fc_tls_close(SOCKET_ID);
  } else {
    fc_tputs("connection error\n");
  }
}

void send_request(char* request) {
  fc_tls_send_chars(SOCKET_ID, request, fc_strlen(request));
  fc_tls_send_chars(SOCKET_ID, CRLF, 2);
}

void handle_default(char* line) {
  fc_tputs("?? ");
  fc_tputs(line);
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
    fc_tputs("\n");
  }
}

void display_page() {
  page_clear_lines(); // erase the current page

  char* line = readline();
  while(line) {
    page_add_line(line);
    if (state.line_count <= 28) {
      screen_redraw();
    } else {
      screen_status();
    }
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

