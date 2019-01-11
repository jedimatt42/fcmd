#include "b3_oem.h"
#include <conio.h>

void defineChars() {
  charsetlc();
  vdpmemcpy(gPattern, &PAT0, 32 * 8);
  vdpmemcpy(gPattern + (127 * 8), &PAT127, 129 * 8);
}

