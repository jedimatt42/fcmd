#include <fc_api.h>
#include <ioports.h>
#include <kscan.h>
#include "gemini.h"
#include "screen.h"
#include "page.h"
#include "mouse.h"

void send_request(char* request);
void handleSuccess(char* line);
void handleDefault(char* line);
void on_exit();

void displayPage();

char CRLF[3] = {'\r', '\n', 0};

#define SOCKET_ID 0

#define VDP_WAIT_VBLANK_CRU	  __asm__( "clr r12\n\ttb 2\n\tjeq -4\n\tmovb @>8802,r12" : : : "r12" );

struct SocketBuffer socketBuffer;

void halt() {
  while(1) { }
}

int fc_main(char* args) {
  fc_strset((char*)&state, 0, sizeof(struct State));
  fc_strncpy(state.url, args, 256);

  init_mouse();
  init_page();
  init_screen();

  if (state.url[0] != 0) {
    open_url(state.url);
  } else {
    fc_ui_gotoxy(0, 2);
    fc_tputs("no url\n");
  }

  while(1) {
    VDP_WAIT_VBLANK_CRU
    int click = update_mouse();
    if (click & MB_LEFT) {
      on_exit();
      return 0;
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
    fc_init_socket_buffer(&socketBuffer, TLS, SOCKET_ID);
    send_request(url);

    char* line = fc_readline(&socketBuffer);
    switch(line[0]) {
      case '2':
	handleSuccess(line);
	break;
      default:
	handleDefault(line);
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

void handleDefault(char* line) {
  fc_tputs("?? ");
  fc_tputs(line);
}

void handleSuccess(char* line) {
  // gobbles the status code, we already know it is a '2x'
  char* tok = fc_strtok(line, ' ');

  tok = fc_strtok(0, ';');
  if (fc_str_startswith(tok, "text/gemini")) {
    displayPage();
  } else if (fc_str_startswith(tok, "text/plain")) {
    displayPage();
  } else {
    fc_tputs("uknown mime-type: ");
    fc_tputs(tok);
    fc_tputs("\n");
  }
}

void displayPage() {
  page_clear_lines(); // erase the current page

  char* line = fc_readline(&socketBuffer);
  while(line) {
    page_add_line(line);
    line = fc_readline(&socketBuffer);
  }
  screen_redraw();
}

