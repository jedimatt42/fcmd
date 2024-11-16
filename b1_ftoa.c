#include "banks.h"
#define MYBANK BANK(1)

void ftoa(char* dest, double a) {
  int* firstword = (int*) &a;

  if (*firstword == 0) {
    dest[0] = '0';
    dest[1] = 0;
    return;
  }

  if (*firstword < 0) {
    *firstword = -(*firstword);
    *dest++ = '-';
  }

  char* r100 = (char*) &a;

  int exponent = 2 * (r100[0] - 64); // unbias the exponent

  int first = r100[1];
  char tens = first / 10;
  char ones = first % 10;

  if (tens) {
    *dest++ = '0' + tens;
    *dest++ = '.';
    exponent++;
  } 
  *dest++ = '0' + ones;
  if (!tens) {
    *dest++ = '.';
  }

  for(int i = 2; i < 8; i++) {
    int digit = r100[i];
    *dest++ = '0' + (digit / 10);
    *dest++ = '0' + (digit % 10);
  }

  *dest++ = 'e';
  if (exponent < 0) {
    *dest++ = '-';
    exponent = -exponent;
  }
  
  char e_hundreds = exponent / 100;
  if (e_hundreds) {
    exponent -= 100;
    *dest++ = '0' + e_hundreds;
  }
  char e_tens = exponent / 10;
  char e_ones = exponent % 10;

  if (e_tens) {
    *dest++ = '0' + e_tens;
  }
  *dest++ = '0' + e_ones;

  *dest++ = 0;
}