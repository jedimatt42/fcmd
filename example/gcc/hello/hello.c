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
    fc_term_puts(fc_hex_from_uint(bytes[i]));
  }
}


int fc_main(char* args) {
  char buffer[64] = "";

  // Checking results against atariage post:
  //   https://forums.atariage.com/topic/366435-max-float/#comment-5507228

  /*
  fc_term_puts("1.020304050607: 4001020304050607 - ");
  print_double_bytes(1.020304050607);
  fc_term_puts("\n");

  fc_term_puts("102.0304050611: 410102030405060B - ");
  print_double_bytes(102.0304050611);
  fc_term_puts("\n");

  fc_term_puts("0.05: 3F05000000000000 - ");
  print_double_bytes(0.05);
  fc_term_puts("\n");

  fc_term_puts("-10.20305050618: BFFF020305050612 - ");
  print_double_bytes(-10.20305050618);
  fc_term_puts("\n");

  fc_term_puts("0.0: 0000000000000000 - ");
  print_double_bytes(0.0);
  fc_term_puts("\n");

  fc_term_puts("ftoa(0): ");
  fc_ftoa(buffer, 0.0);
  fc_term_puts(buffer);

  fc_term_puts("\nftoa(1.0): ");
  fc_ftoa(buffer, 1.0);
  fc_term_puts(buffer);

  fc_term_puts("\nftoa(-1.0): ");
  fc_ftoa(buffer, -1.0);
  fc_term_puts(buffer);

    fc_term_puts("\nftoa(10.0): ");
  fc_ftoa(buffer, 10.0);
  fc_term_puts(buffer);

  fc_term_puts("\nftoa(-10.0): ");
  fc_ftoa(buffer, -10.0);
  fc_term_puts(buffer);

  fc_term_puts("\nftoa(100.3): ");
  fc_ftoa(buffer, 100.3);
  fc_term_puts(buffer);

  fc_term_puts("\nftoa(-100.3): ");
  fc_ftoa(buffer, -100.3);
  fc_term_puts(buffer);

  fc_term_puts("\nftoa(1234.3): ");
  fc_ftoa(buffer, 1234.3);
  fc_term_puts(buffer);

  fc_term_puts("\nftoa(-1234.3): ");
  fc_ftoa(buffer, -1234.3);
  fc_term_puts(buffer);

  fc_term_puts("\nftoa(12345.3): ");
  fc_ftoa(buffer, 12345.3);
  fc_term_puts(buffer);

  fc_term_puts("\nftoa(-12345.3): ");
  fc_ftoa(buffer, -12345.3);
  fc_term_puts(buffer);

  fc_term_puts("\nftoa(123456.3): ");
  fc_ftoa(buffer, 123456.3);
  fc_term_puts(buffer);

  fc_term_puts("\nftoa(-123456.3): ");
  fc_ftoa(buffer, -123456.3);
  fc_term_puts(buffer);

  fc_term_puts("\n");
  */

  // a = 1.3;
  // b = a / 2;
  // c = 1.3;

  /*
  // <
  if ( a < b ) {
    fc_term_puts("FAIL: A < B\n");
  }
  if ( b < a ) {
    fc_term_puts("PASS: b < a\n");
  }
  // <= 
  if ( a <= b ) { 
    fc_term_puts("FAIL: A <= B\n");
  }
  if ( b <= a ) {
    fc_term_puts("PASS: b <= a\n");
  }
  // >
  if ( b > a ) {
    fc_term_puts("FAIL: b > a\n");
  }
  if ( a > b ) {
    fc_term_puts("PASS: a > b\n");
  }
  // >= 
  if ( b >= a ) {
    fc_term_puts("FAIL: b >= a\n");
  }
  if ( a >= b ) {
    fc_term_puts("PASS: a >= b\n");
  }
  // ==
  if ( a == b ) {
    fc_term_puts("FAIL: a == b\n");
  }
  if ( a == c ) {
    fc_term_puts("PASS: a == c\n");
  }
  // != 
  if ( a != c ) {
    fc_term_puts("FAIL: a != c\n");
  }
  if ( a != b ) {
    fc_term_puts("PASS: a != b\n");
  }
  // negate
  c = -a;
  if ( c == a ) {
    fc_term_puts("FAIL: c(-1.3) == a(1.3)\n");
  }
  if ( c != a ) {
    fc_term_puts("PASS: c(-1.3) != a(1.3)\n");
  }
  */

  // fc_term_puts("-- int to float & float to int conversion -\n");
  // c = takesint(-3, 3);
  // fc_ftoa(buffer, c);
  // fc_term_puts("takesint(-3, 3) = -27: ");
  // fc_term_puts(buffer);
  // fc_term_puts("\n");
  // print_double_bytes(c);
  // fc_term_puts("\n");

  // c = takesint(2, 8);
  // fc_ftoa(buffer, c);
  // fc_term_puts("takesint(2, 8) = 256: ");
  // fc_term_puts(buffer);
  // fc_term_puts("\n");
  // print_double_bytes(c);
  // fc_term_puts("\n");

  // c = takesint(5, 3);
  // fc_ftoa(buffer, c);
  // fc_term_puts("takesint(5, 3) = 125: ");
  // fc_term_puts(buffer);
  // fc_term_puts("\n");
  // print_double_bytes(c);
  // fc_term_puts("\n");

  char input[80];
  input[0] = 0;
  while(input[0] != 'q') {
    double value = 0;
    int intvalue = 0;
    fc_term_puts("enter an integer:\n");
    fc_str_set(input, 0, 80);
    fc_term_gets(input, 80, 1);
    fc_term_puts("\nuser input: ");
    fc_term_puts(input);
    // tireal_atof(input, (unsigned char*) &value);
    intvalue = fc_str_to_int(input);
    fc_term_puts("\nas int: ");
    fc_term_puts(fc_str_from_uint(intvalue));
    fc_term_puts("\nas float: ");
    value = (double) intvalue;
    fc_term_puts("\n  bytes: ");
    print_double_bytes(value);
    fc_term_puts("\n  float string: ");
    fc_ftoa(buffer, value);
    fc_term_puts(buffer);
    fc_term_puts("\n");
  }

  return 0;
}
