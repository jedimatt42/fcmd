#include "banks.h"
#define MYBANK BANK(2)

#include "b2_tifloat.h"
#include "b8_terminal.h"
#include "string.h"

#define _USE_FAC 1

#ifndef _USE_FAC

static void __attribute__((noinline)) twosComplement(char* bytes) {
  unsigned int word0 = ((unsigned int)bytes[1]) << 8;
  word0 |= ((unsigned int)bytes[2]);
  word0 = ~word0;
  word0++;
  bytes[1] = (word0 >> 8);
  bytes[2] = (word0 & 0xff);
}

int ti_floatToInt(char* bytes) {
  int signbit = bytes[1] & 0x80;

  if (signbit) {
    twosComplement(bytes);
  }

  int exp = *(bytes+1);
  exp = exp & 0x7f;
  exp -= 64;
  int result = bytes[2];
  int idx = 3;

  while(exp > 0) {
    result = result * 100;
    result += (unsigned int) bytes[idx];
    idx++;
    exp--;
  }

  if (signbit) {
    return -result;
  } else {
    return result;
  }
}

#endif


// Use routine in console rom that converts numbers in the FAC portion of scratchpad ram
#ifdef _USE_FAC


#define FAC ((char*)0x834A)

int ti_floatToInt(char* bytes) {
  // copy to FAC in scratchpad ram - byte[0] is the BASIC data type byte (8)
  memcpy(FAC, bytes+1, 8);

  // set GPLWS, run console rom CFI routine, restore gcc WS
  __asm__(
    "lwpi >83E0\n\t"
    "bl @>12b8\n\t"
    "lwpi >8300\n\t"
  );

  return *((int*)FAC);
}

#endif
