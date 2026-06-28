#include "banks.h"
#define MYBANK BANK(1)

// Uses the existing ftoa helper to convert double to string.
// Returns pointer to a static buffer (like str_from_uint).
// Not thread safe, don't use from interrupt handlers.
char* str_from_float(double a) {
  static char buf[80];
  extern void ftoa(char* dest, double a);
  ftoa(buf, a);
  return buf;
}
