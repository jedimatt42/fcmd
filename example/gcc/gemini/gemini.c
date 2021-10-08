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

void displayPage();

char CRLF[3] = {'\r', '\n', 0};

#define SOCKET_ID 0

#define VDP_WAIT_VBLANK_CRU	  __asm__( "clr r12\n\ttb 2\n\tjeq -4\n\tmovb @>8802,r12" : : : "r12" );

struct SocketBuffer socketBuffer;

int fc_main(char* args) {
  init_mouse();
  init_page();
  init_screen();

  if (args[0] != 0) {
    open_url(args);
  }

  fc_tputs("click to exit\n");
  while(1) {
    VDP_WAIT_VBLANK_CRU
    int click = update_mouse();
    if (click & MB_LEFT) {
      return 0;
    }
  }
}

void open_url(char* url) {
  char hostname[80];
  char port[10];
  if (fc_str_startswith(url, "gemini://")) {
    int i = fc_indexof(url + 9, '/');
    if (i == -1) {
      fc_strcpy(hostname, url + 9);
    } else {
      fc_strncpy(hostname, url + 9, i);
    }
    fc_tputs("hostname: ");
    fc_tputs(hostname);
    fc_tputs("\n");

    i = fc_indexof(hostname, ':');
    if (i == -1) {
      fc_strcpy(port, "1965");
    } else {
      fc_strcpy(port, hostname + i + 1);
      hostname[i] = 0;
    }
    fc_tputs("port: ");
    fc_tputs(port);
    fc_tputs("\n");

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
}

void send_request(char* request) {
  fc_tls_send_chars(SOCKET_ID, request, fc_strlen(request));
  fc_tls_send_chars(SOCKET_ID, CRLF, 2);
}

void handleDefault(char* line) {
  fc_tputs("??\n");
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
  char* line = fc_readline(&socketBuffer);
  while(line) {
    fc_tputs(line);
    line = fc_readline(&socketBuffer);
  }
}

