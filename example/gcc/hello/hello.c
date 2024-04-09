#include <fc_api.h>

volatile float a = 0;
volatile float b = 0;
volatile float c = 0;

volatile int ia = 0;
volatile int ib = 0;
volatile int ic = 0;

void tireal_ftoa (float *d, char *p);

double pow(double x, double y);

double pow(double x, double y) {
  if ( y == 0 ) {
    return 1;
  }
  int result = x; // if y is 1, result is just x;
  // if y is greater than 1, that multiply repeatedly
  for (int i = 1; i < y; i++) {
    result = result * x;
  }
  return result;
}

int fc_main(char* args) {
  fc_tputs("Hello\n");
  a = 1.3;
  b = a / 2;
  c = 1.3;

  // <
  if ( a < b ) {
    fc_tputs("FAIL: A < B\n");
  }
  if ( b < a ) {
    fc_tputs("b < a\n");
  }
  // <= 
  if ( a <= b ) { 
    fc_tputs("FAIL: A <= B\n");
  }
  if ( b <= a ) {
    fc_tputs("b <= a\n");
  }
  // >
  if ( b > a ) {
    fc_tputs("FAIL: b > a\n");
  }
  if ( a > b ) {
    fc_tputs("a > b\n");
  }
  // >= 
  if ( b >= a ) {
    fc_tputs("FAIL: b >= a\n");
  }
  if ( a >= b ) {
    fc_tputs("a >= b\n");
  }
  // ==
  if ( a == b ) {
    fc_tputs("FAIL: a == b\n");
  }
  if ( a == c ) {
    fc_tputs("a == c\n");
  }
  // != 
  if ( a != c ) {
    fc_tputs("FAIL: a != c\n");
  }
  if ( a != b ) {
    fc_tputs("a != b\n");
  }
  c = -a;
  if ( c == a ) {
    fc_tputs("-a (c) == a\n");
  }
  char strbuf[30];
  tireal_ftoa((float*)&c, strbuf);
  fc_tputs(strbuf);
  float d = 1.4567;
  tireal_ftoa(&d, strbuf);
  fc_tputs(strbuf);
  fc_tputs("\n");

  fc_tputs("---\n");
  c = pow(3, 2);
  tireal_ftoa((float*)&c, strbuf);
  fc_tputs(strbuf);
  fc_tputs("\n");

  return 0;
}
