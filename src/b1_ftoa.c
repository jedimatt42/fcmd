#include "banks.h"
#define MYBANK BANK(1)

void ftoa(char* dest, double a) {
  // Copy the double's 8 bytes to a local buffer to avoid destructive negation.
  // TMS9900 is big-endian 16-bit, so int is 2 bytes; 4 ints = 8 bytes.
  char buf[8];
  {
    int* src = (int*)&a;
    int* dst = (int*)buf;
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
  }
  char* r100 = buf;

  // Zero: first word (exponent + first mantissa byte) is all zero
  if (((int*)buf)[0] == 0) {
    dest[0] = '0';
    dest[1] = 0;
    return;
  }

  // Negative: negate first word on the local copy, emit '-'
  if (((int*)buf)[0] < 0) {
    ((int*)buf)[0] = -((int*)buf)[0];
    *dest++ = '-';
  }

  int rawExp = 2 * (r100[0] - 64);  // radix-100 → decimal exponent

  int first = r100[1];
  char tens = first / 10;
  char ones = first % 10;

  // Unpack the 14 mantissa digits into a char array.
  // When tens is non-zero the leading digit is significant and
  // effective exponent is raw + 1.
  int effectiveExp = rawExp;
  char digits[15];
  int n = 0;
  if (tens) {
    digits[n++] = '0' + tens;
    digits[n++] = '0' + ones;
    effectiveExp++;
  } else {
    digits[n++] = '0' + ones;
  }
  for (int i = 2; i < 8; i++) {
    int d = r100[i];
    digits[n++] = '0' + (d / 10);
    digits[n++] = '0' + (d % 10);
  }

  // Strip trailing zeros
  int lastSig = n - 1;
  while (lastSig >= 0 && digits[lastSig] == '0') {
    lastSig--;
  }

  // Decimal point position = effectiveExp + 1 (number of digits before decimal)
  int decPos = effectiveExp + 1;

  if (effectiveExp >= 0 && decPos <= 14) {
    // Decimal form: point falls within or at the end of digit range
    int intEnd = decPos - 1;  // last digit before decimal (0-indexed)
    if (intEnd < lastSig) {
      for (int i = 0; i <= intEnd; i++) *dest++ = digits[i];
      *dest++ = '.';
      for (int i = intEnd + 1; i <= lastSig; i++) *dest++ = digits[i];
    } else {
      for (int i = 0; i <= lastSig; i++) *dest++ = digits[i];
      for (int i = 0; i < intEnd - lastSig; i++) *dest++ = '0';
    }
  } else if (effectiveExp < 0 && effectiveExp >= -6) {
    // Small number: "0.000...digits"
    *dest++ = '0';
    *dest++ = '.';
    for (int i = 0; i < -effectiveExp - 1; i++) *dest++ = '0';
    for (int i = 0; i <= lastSig; i++) *dest++ = digits[i];
  } else {
    // Scientific notation (very large or very small)
    *dest++ = digits[0];
    if (lastSig > 0) {
      *dest++ = '.';
      for (int i = 1; i <= lastSig; i++) *dest++ = digits[i];
    }
    *dest++ = 'e';
    if (effectiveExp < 0) {
      *dest++ = '-';
      effectiveExp = -effectiveExp;
    }
    char et = effectiveExp / 10;
    char eo = effectiveExp % 10;
    if (et) *dest++ = '0' + et;
    *dest++ = '0' + eo;
  }

  *dest = 0;
}
