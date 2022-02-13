#include <fc_api.h>

#include "tifiles.h"

int isTiFiles(struct TiFiles* tifiles) {
  char buf[11];
  char* raw = (char*) tifiles;
  if (raw[0] != 7) {
    return 0;
  }
  fc_basicToCstr(raw, buf);
  return !fc_strcmp(buf, "TIFILES");
}

