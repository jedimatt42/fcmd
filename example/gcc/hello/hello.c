#include <fc_api.h>

void tireal_atof (const char *a, unsigned char *result);

double __floatsidf(int a);

volatile float a = 0;
volatile float b = 0;
volatile float c = 0;

float __attribute__((noinline)) takesint(int a, int y) {
  return (float) a;
}

void __attribute__((noinline)) print_double_bytes(double a) {
  int* bytes = (int*) &a;
  for (int i = 0; i < 4; i++) {
    fc_tputs(fc_uint2hex(bytes[i]));
  }
}


int fc_main(char* args) {
  char buffer[64] = "";

  // Checking results against atariage post:
  //   https://forums.atariage.com/topic/366435-max-float/#comment-5507228

  /*
  fc_tputs("1.020304050607: 4001020304050607 - ");
  print_double_bytes(1.020304050607);
  fc_tputs("\n");

  fc_tputs("102.0304050611: 410102030405060B - ");
  print_double_bytes(102.0304050611);
  fc_tputs("\n");

  fc_tputs("0.05: 3F05000000000000 - ");
  print_double_bytes(0.05);
  fc_tputs("\n");

  fc_tputs("-10.20305050618: BFFF020305050612 - ");
  print_double_bytes(-10.20305050618);
  fc_tputs("\n");

  fc_tputs("0.0: 0000000000000000 - ");
  print_double_bytes(0.0);
  fc_tputs("\n");

  fc_tputs("ftoa(0): ");
  fc_ftoa(buffer, 0.0);
  fc_tputs(buffer);

  fc_tputs("\nftoa(1.0): ");
  fc_ftoa(buffer, 1.0);
  fc_tputs(buffer);

  fc_tputs("\nftoa(-1.0): ");
  fc_ftoa(buffer, -1.0);
  fc_tputs(buffer);

    fc_tputs("\nftoa(10.0): ");
  fc_ftoa(buffer, 10.0);
  fc_tputs(buffer);

  fc_tputs("\nftoa(-10.0): ");
  fc_ftoa(buffer, -10.0);
  fc_tputs(buffer);

  fc_tputs("\nftoa(100.3): ");
  fc_ftoa(buffer, 100.3);
  fc_tputs(buffer);

  fc_tputs("\nftoa(-100.3): ");
  fc_ftoa(buffer, -100.3);
  fc_tputs(buffer);

  fc_tputs("\nftoa(1234.3): ");
  fc_ftoa(buffer, 1234.3);
  fc_tputs(buffer);

  fc_tputs("\nftoa(-1234.3): ");
  fc_ftoa(buffer, -1234.3);
  fc_tputs(buffer);

  fc_tputs("\nftoa(12345.3): ");
  fc_ftoa(buffer, 12345.3);
  fc_tputs(buffer);

  fc_tputs("\nftoa(-12345.3): ");
  fc_ftoa(buffer, -12345.3);
  fc_tputs(buffer);

  fc_tputs("\nftoa(123456.3): ");
  fc_ftoa(buffer, 123456.3);
  fc_tputs(buffer);

  fc_tputs("\nftoa(-123456.3): ");
  fc_ftoa(buffer, -123456.3);
  fc_tputs(buffer);

  fc_tputs("\n");
  */

  // a = 1.3;
  // b = a / 2;
  // c = 1.3;

  /*
  // <
  if ( a < b ) {
    fc_tputs("FAIL: A < B\n");
  }
  if ( b < a ) {
    fc_tputs("PASS: b < a\n");
  }
  // <= 
  if ( a <= b ) { 
    fc_tputs("FAIL: A <= B\n");
  }
  if ( b <= a ) {
    fc_tputs("PASS: b <= a\n");
  }
  // >
  if ( b > a ) {
    fc_tputs("FAIL: b > a\n");
  }
  if ( a > b ) {
    fc_tputs("PASS: a > b\n");
  }
  // >= 
  if ( b >= a ) {
    fc_tputs("FAIL: b >= a\n");
  }
  if ( a >= b ) {
    fc_tputs("PASS: a >= b\n");
  }
  // ==
  if ( a == b ) {
    fc_tputs("FAIL: a == b\n");
  }
  if ( a == c ) {
    fc_tputs("PASS: a == c\n");
  }
  // != 
  if ( a != c ) {
    fc_tputs("FAIL: a != c\n");
  }
  if ( a != b ) {
    fc_tputs("PASS: a != b\n");
  }
  // negate
  c = -a;
  if ( c == a ) {
    fc_tputs("FAIL: c(-1.3) == a(1.3)\n");
  }
  if ( c != a ) {
    fc_tputs("PASS: c(-1.3) != a(1.3)\n");
  }
  */

  // fc_tputs("-- int to float & float to int conversion -\n");
  // c = takesint(-3, 3);
  // fc_ftoa(buffer, c);
  // fc_tputs("takesint(-3, 3) = -27: ");
  // fc_tputs(buffer);
  // fc_tputs("\n");
  // print_double_bytes(c);
  // fc_tputs("\n");

  // c = takesint(2, 8);
  // fc_ftoa(buffer, c);
  // fc_tputs("takesint(2, 8) = 256: ");
  // fc_tputs(buffer);
  // fc_tputs("\n");
  // print_double_bytes(c);
  // fc_tputs("\n");

  // c = takesint(5, 3);
  // fc_ftoa(buffer, c);
  // fc_tputs("takesint(5, 3) = 125: ");
  // fc_tputs(buffer);
  // fc_tputs("\n");
  // print_double_bytes(c);
  // fc_tputs("\n");

  char input[80];
  input[0] = 0;
  while(input[0] != 'q') {
    double value = 0;
    int intvalue = 0;
    fc_tputs("enter an integer:\n");
    fc_strset(input, 0, 80);
    fc_getstr(input, 80, 1);
    fc_tputs("\nuser input: ");
    fc_tputs(input);
    // tireal_atof(input, (unsigned char*) &value);
    intvalue = fc_atoi(input);
    fc_tputs("\nas int: ");
    fc_tputs(fc_uint2str(intvalue));
    fc_tputs("\nas float: ");
    value = (double) intvalue;
    fc_tputs("\n  bytes: ");
    print_double_bytes(value);
    fc_tputs("\n  float string: ");
    fc_ftoa(buffer, value);
    fc_tputs(buffer);
    fc_tputs("\n");
  }

  return 0;
}
