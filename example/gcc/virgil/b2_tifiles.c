#include <fc_api.h>

#include "tifiles.h"

int isTiFiles(struct TiFiles* tifiles) {
  char buf[11];
  char* raw = (char*) tifiles;
  if (raw[0] != 7) {
    return 0;
  }
  str_from_basic(raw, buf);
  return !str_cmp(buf, "TIFILES");
}

