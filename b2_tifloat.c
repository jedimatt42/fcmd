#include "banks.h"
#define MYBANK BANK(2)

#include "b2_tifloat.h"
#include "b8_terminal.h"

int ti_floatToInt(unsigned char* bytes) {
  if (bytes[0] == 8) {
    int signbit = bytes[1] & 0x80;

    if (signbit) {
      unsigned int word0 = ~(((unsigned int)bytes[1]) << 8 | bytes[2]) + 1;
      bytes[1] = word0 & 0x7f00 >> 8;
      bytes[2] = word0 & 0xff;
    }

    int exp = (bytes[1] & 0x7f) - 64;
    int result = bytes[2];
    int idx = 3;
    if (exp > 2) {
      tputs_rom("[Uh oh]");
    }
    while(exp) {
      result = result * 100;
      result += bytes[idx];
      idx++;
      exp--;
    }
    return signbit ? -1 * result : result;
  } else {
    return 0;
  }
}
