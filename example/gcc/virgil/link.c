#include <fc_api.h>
#include "link.h"

static char buf[256];

char* link_url(char* line, int* length) {
  int i = 3;
  // format is '=>' whitespace url whitespace label
  while(line[i] == ' ' || line[i] == '\t') {
    // consume whitespace
    i++;
  }
  int b = 0;
  while(line[i] != ' ' && line[i] != '\t' && line[i] != 0) {
    // copy url
    buf[b++] = line[i++];
  }
  buf[b] = 0;
  *length = b;
  return buf;
}

static int is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

char* link_label(char* line, int* length) {
  buf[0] = '=';
  buf[1] = '>';
  buf[2] = ' ';
  int i = 3;
  // format is '=>' whitespace url whitespace label
  while(is_whitespace(line[i])) {
    // consume whitespace
    i++;
  }
  int b = 3;
  while((!is_whitespace(line[i])) && line[i] != 0) {
    // copy url in case there is no label
    buf[b++] = line[i++];
  }
  buf[b] = 0;
  if (line[i] != 0) {
    // consume whitespace
    while(is_whitespace(line[i])) {
      i++;
    }
    if (line[i] != 0) {
      // copy remainder of line as label
      b = 3;
      while(line[i] != 0 && line[i] != '\r' && line[i] != '\n') {
	buf[b++] = line[i++];
      }
      buf[b] = 0;
    }
  }

  *length = b;
  return buf;
}

void update_full_url(char* dst, char* url) {
  char tmp[256];
  if (fc_str_startswith(url, "gemini://")) {
    // a little wasteful, but good enough.
    fc_strcpy(tmp, url);
  } else if (fc_str_startswith(url, "//")) {
    // implicit gemini: protocol
    fc_strcpy(tmp, "gemini:");
    fc_strcpy(tmp + 7, url);
  } else if (fc_str_startswith(url, "/")) {
    // same host and port, new path
    char hostname[80];
    char port[8];
    // assumes dst is also previous url
    set_hostname_and_port(dst, hostname, port);
    fc_strcpy(tmp, "gemini://");
    int len = 9;
    fc_strcpy(tmp + len, hostname);
    len += fc_strlen(hostname);
    fc_strcpy(tmp + len, url);
  }
  fc_strcpy(dst, tmp);
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

