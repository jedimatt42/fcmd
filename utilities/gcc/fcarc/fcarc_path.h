#ifndef _FCARC_PATH_H
#define _FCARC_PATH_H 1

#include <fc_api.h>

static inline int path_last_dot(const char* s) {
  int i = 0;
  int last = -1;
  while (s[i]) {
    if (s[i] == '.') {
      last = i;
    }
    i++;
  }
  return last;
}

/* Split "DEV.DIR.NAME" into parent "DEV.DIR." and name "NAME".
 * parent must hold at least 256 bytes. name must hold at least 16 bytes. */
static inline int path_split(const char* full, char* parent, char* name) {
  int len = str_len(full);
  int dot = path_last_dot(full);
  if (dot < 0) {
    parent[0] = 0;
    str_ncopy(name, (char*)full, 15);
    name[15] = 0;
    return 0;
  }
  int i;
  for (i = 0; i <= dot; i++) {
    parent[i] = full[i];
  }
  parent[i] = 0;
  int j = 0;
  i = dot + 1;
  while (full[i] && j < 15) {
    name[j++] = full[i++];
  }
  name[j] = 0;
  return 0;
}

/* Last component, ignoring one trailing dot. "TIPI.GAMES." -> "GAMES". */
static inline int path_component(const char* full, char* out, int max) {
  int len = str_len(full);
  while (len > 0 && full[len - 1] == '.') {
    len--;
  }
  int start = len;
  while (start > 0 && full[start - 1] != '.') {
    start--;
  }
  int n = len - start;
  if (n <= 0 || n >= max) {
    return -1;
  }
  int i;
  for (i = 0; i < n; i++) {
    out[i] = full[start + i];
  }
  out[n] = 0;
  return 0;
}

/* Append a component to a path. base may be empty or end in '.'. */
static inline int path_append(char* dst, const char* base, const char* comp, int max) {
  int blen = str_len(base);
  int clen = str_len(comp);
  int need = blen + clen + 2;
  if (need > max) {
    return -1;
  }
  str_copy(dst, base);
  int pos = blen;
  if (pos > 0 && dst[pos - 1] != '.') {
    dst[pos++] = '.';
  }
  int i;
  for (i = 0; i < clen; i++) {
    dst[pos++] = comp[i];
  }
  dst[pos] = 0;
  return 0;
}

/* Join a base directory and a dot-separated archive-relative path. */
static inline int path_join_rel(char* dst, const char* base, const char* rel, int max) {
  int blen = str_len(base);
  int rlen = str_len(rel);
  int need = blen + rlen + 2;
  if (need > max) {
    return -1;
  }
  str_copy(dst, base);
  int pos = blen;
  if (pos > 0 && dst[pos - 1] != '.') {
    dst[pos++] = '.';
  }
  int i;
  for (i = 0; i < rlen; i++) {
    dst[pos++] = rel[i];
  }
  dst[pos] = 0;
  return 0;
}

/* Check one archive-relative path for TI compatibility. */
static inline int path_validate(const char* p) {
  int len = str_len(p);
  if (len < 1 || len > MAX_PATH) {
    return 0;
  }
  int comp = 0;
  int i;
  for (i = 0; i <= len; i++) {
    char c = (i == len) ? '.' : p[i];
    if (c == '.') {
      if (comp == 0 || comp > MAX_COMPONENT) {
        return 0;
      }
      comp = 0;
    } else {
      if (c == ' ' || c < 32) {
        return 0;
      }
      comp++;
    }
  }
  return 1;
}

#endif
