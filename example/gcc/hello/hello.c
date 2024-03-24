#include <fc_api.h>

volatile float a = 0;
volatile float b = 0;
volatile float c = 0;

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
  return 0;
}
