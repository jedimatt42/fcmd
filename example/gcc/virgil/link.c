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

