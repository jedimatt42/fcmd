#include <fc_api.h>
#include "link.h"
#include "page.h"

static char buf[256];

void FC_SAMS(1,link_set_url(char* dst, int line_id)) {
  // the 'normalize_url' code needs pretty clean buffer
  fc_strset(dst, 0, 256);
  // find first line of link
  int idx = line_id;
  struct Line* link_line = page_get_line(idx);
  while(link_line->type == LINE_TYPE_LINK_CONT) {
    link_line = page_get_line(--idx);
  }

  // now link_line and idx reference the first part of the link.
  
  // format in page line is '=>' [whitespace] url [<whitespace> label]
  int i = 2;
  char* line = link_line->data;

  while(line[i] == ' ' || line[i] == '\t') {
    // consume optional whitespace
    i++;
  }
  int b = 0;
  while(i < 80 && line[i] != ' ' && line[i] != '\t' && line[i] != 0) {
   dst[b++] = line[i++];
  }
  link_line = page_get_line(++idx);
  int done = 0;
  while (link_line->type == LINE_TYPE_LINK_CONT && !done) {
    line = link_line->data;
    i = 3; // we control the '-> ' prefix
    while(b < 255 && i < 80 && line[i] != ' ' && line[i] != '\t' && line[i] != 0) {
      dst[b++] = line[i++];
    }
    if (i < 80) {
      done = 1;
    }
    link_line = page_get_line(++idx);
  }
  // todo: investigate subsequent lines if we hit the limit
  buf[b] = 0;
}

char* FC_SAMS(1,link_url_scheme(char* line, int* length)) {
  int i = 2;
  // format is '=>' [whitespace] url [<whitespace> label]
  while(line[i] == ' ' || line[i] == '\t') {
    // consume optional whitespace
    i++;
  }
  int b = 0;
  while(b < 10 && line[i] != ':' && line[i] != ' ' && line[i] != '\t' && line[i] != 0) {
    // copy scheme
    buf[b++] = line[i++];
  }
  if (line[i] == ':') {
    buf[b] = 0;
    *length = b;
  } else {
    // default to gemini scheme
    fc_strcpy(buf, "gemini");
    *length = 6;
  }
  return buf;
}

static int is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

char* FC_SAMS(1,link_label(char* line, int* length)) {
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

void FC_SAMS(1,update_full_url(char* dst, char* url)) {
  char tmp[256];
  if (fc_str_startswith(url, "gemini://")) {
    // a little wasteful, but good enough.
    fc_strcpy(tmp, url);
  } else if (url[0] == '/' && url[1] == '/') {
    // implicit gemini: protocol
    fc_strcpy(tmp, "gemini:");
    fc_strcpy(tmp + 7, url);
  } else if (url[0] == '/') {
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
  } else {
    fc_strset(tmp, 0, 256);
    int l = 0;
    int slashes = 0;
    int lastSlash = 0;
    // find the index of the last slash in previous url
    // while copying into tmp
    while(dst[l] != 0) {
      tmp[l] = dst[l];
      if (tmp[l] == '/') {
	slashes++;
	lastSlash = l;
      }
      l++;
    }
    if (slashes < 3) {
      tmp[l++] = '/';
    } else {
      l = lastSlash + 1;
    }
    fc_strncpy(tmp + l, url, 255 - l);
  }
  fc_strcpy(dst, tmp);
}

void FC_SAMS(1,normalize_url(char* url)) {
  char dst[256];

  int u = 0;
  int d = 0;
  int slashes = 0;
  int parent = 0;
  while(u < 256 && url[u] != 0) {
    dst[d++] = url[u++];
    if (dst[d-1] == '/' || dst[d-1] == 0) {
      if (dst[d-2] == '.' && dst[d-3] == '/') {
	// replace /./ with /
	// replace /.$ with /
	d -= 2;
	dst[d] = 0;
      } else if (parent != 0 && dst[d-2] == '.' && dst[d-3] == '.' && dst[d-4] == '/') {
	// replace /term/../ with /
	dst[parent] = 0;
	d = parent;
      } else if (dst[d-1] == '/' && url[u] != '.') {
	slashes++;
	if (slashes >= 3) {
	  parent = d;
	}
      }
    }
  }
  dst[d] = 0;
  fc_strncpy(url, dst, 256);
}

void FC_SAMS(1,set_hostname_and_port(char* url, char* hostname, char* port)) {
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

