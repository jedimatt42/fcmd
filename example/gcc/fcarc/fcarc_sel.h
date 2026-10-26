#ifndef _FCARC_SEL_H
#define _FCARC_SEL_H 1

#include <fc_api.h>
#include "fcarc.h"
#include "fcarc_util.h"

static inline int sel_path_matches(struct SelSet* s, const char* path) {
  if (s->count == 0) {
    return 1;
  }
  int i;
  for (i = 0; i < s->count; i++) {
    if (ci_path_match(path, s->item[i])) {
      return 1;
    }
  }
  return 0;
}

static inline int sel_dir_wanted(struct SelSet* s, const char* path) {
  if (s->count == 0) {
    return 1;
  }
  int i;
  for (i = 0; i < s->count; i++) {
    if (ci_path_match(path, s->item[i]) || ci_path_match(s->item[i], path)) {
      return 1;
    }
  }
  return 0;
}

static inline void sel_parse(char* rest, struct SelSet* sel) {
  sel->count = 0;
  char tok[SELECT_LEN];
  while (rest && rest[0] && sel->count < MAX_SELECT) {
    rest = str_token_next(tok, rest, ' ');
    str_ncopy(sel->item[sel->count], tok, SELECT_LEN - 1);
    sel->item[sel->count][SELECT_LEN - 1] = 0;
    sel->count++;
  }
}

#endif
