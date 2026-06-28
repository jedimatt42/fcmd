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
    term_puts(hex_from_uint(bytes[i]));
  }
}


int main(char* args) {
  char buffer[64] = "";

  // Checking results against atariage post:
  //   https://forums.atariage.com/topic/366435-max-float/#comment-5507228

  /*
  term_puts("1.020304050607: 4001020304050607 - ");
  print_double_bytes(1.020304050607);
  term_puts("\n");

  term_puts("102.0304050611: 410102030405060B - ");
  print_double_bytes(102.0304050611);
  term_puts("\n");

  term_puts("0.05: 3F05000000000000 - ");
  print_double_bytes(0.05);
  term_puts("\n");

  term_puts("-10.20305050618: BFFF020305050612 - ");
  print_double_bytes(-10.20305050618);
  term_puts("\n");

  term_puts("0.0: 0000000000000000 - ");
  print_double_bytes(0.0);
  term_puts("\n");

  term_puts("ftoa(0): ");
  fc_ftoa(buffer, 0.0);
  term_puts(buffer);

  term_puts("\nftoa(1.0): ");
  fc_ftoa(buffer, 1.0);
  term_puts(buffer);

  term_puts("\nftoa(-1.0): ");
  fc_ftoa(buffer, -1.0);
  term_puts(buffer);

    term_puts("\nftoa(10.0): ");
  fc_ftoa(buffer, 10.0);
  term_puts(buffer);

  term_puts("\nftoa(-10.0): ");
  fc_ftoa(buffer, -10.0);
  term_puts(buffer);

  term_puts("\nftoa(100.3): ");
  fc_ftoa(buffer, 100.3);
  term_puts(buffer);

  term_puts("\nftoa(-100.3): ");
  fc_ftoa(buffer, -100.3);
  term_puts(buffer);

  term_puts("\nftoa(1234.3): ");
  fc_ftoa(buffer, 1234.3);
  term_puts(buffer);

  term_puts("\nftoa(-1234.3): ");
  fc_ftoa(buffer, -1234.3);
  term_puts(buffer);

  term_puts("\nftoa(12345.3): ");
  fc_ftoa(buffer, 12345.3);
  term_puts(buffer);

  term_puts("\nftoa(-12345.3): ");
  fc_ftoa(buffer, -12345.3);
  term_puts(buffer);

  term_puts("\nftoa(123456.3): ");
  fc_ftoa(buffer, 123456.3);
  term_puts(buffer);

  term_puts("\nftoa(-123456.3): ");
  fc_ftoa(buffer, -123456.3);
  term_puts(buffer);

  term_puts("\n");
  */

  // a = 1.3;
  // b = a / 2;
  // c = 1.3;

  /*
  // <
  if ( a < b ) {
    term_puts("FAIL: A < B\n");
  }
  if ( b < a ) {
    term_puts("PASS: b < a\n");
  }
  // <= 
  if ( a <= b ) { 
    term_puts("FAIL: A <= B\n");
  }
  if ( b <= a ) {
    term_puts("PASS: b <= a\n");
  }
  // >
  if ( b > a ) {
    term_puts("FAIL: b > a\n");
  }
  if ( a > b ) {
    term_puts("PASS: a > b\n");
  }
  // >= 
  if ( b >= a ) {
    term_puts("FAIL: b >= a\n");
  }
  if ( a >= b ) {
    term_puts("PASS: a >= b\n");
  }
  // ==
  if ( a == b ) {
    term_puts("FAIL: a == b\n");
  }
  if ( a == c ) {
    term_puts("PASS: a == c\n");
  }
  // != 
  if ( a != c ) {
    term_puts("FAIL: a != c\n");
  }
  if ( a != b ) {
    term_puts("PASS: a != b\n");
  }
  // negate
  c = -a;
  if ( c == a ) {
    term_puts("FAIL: c(-1.3) == a(1.3)\n");
  }
  if ( c != a ) {
    term_puts("PASS: c(-1.3) != a(1.3)\n");
  }
  */

  // term_puts("-- int to float & float to int conversion -\n");
  // c = takesint(-3, 3);
  // fc_ftoa(buffer, c);
  // term_puts("takesint(-3, 3) = -27: ");
  // term_puts(buffer);
  // term_puts("\n");
  // print_double_bytes(c);
  // term_puts("\n");

  // c = takesint(2, 8);
  // fc_ftoa(buffer, c);
  // term_puts("takesint(2, 8) = 256: ");
  // term_puts(buffer);
  // term_puts("\n");
  // print_double_bytes(c);
  // term_puts("\n");

  // c = takesint(5, 3);
  // fc_ftoa(buffer, c);
  // term_puts("takesint(5, 3) = 125: ");
  // term_puts(buffer);
  // term_puts("\n");
  // print_double_bytes(c);
  // term_puts("\n");

  char input[80];
  input[0] = 0;
  while(input[0] != 'q') {
    double value = 0;
    int intvalue = 0;
    term_puts("enter an integer:\n");
    str_set(input, 0, 80);
    term_gets(input, 80, 1);
    term_puts("\nuser input: ");
    term_puts(input);
    // tireal_atof(input, (unsigned char*) &value);
    intvalue = str_to_int(input);
    term_puts("\nas int: ");
    term_puts(str_from_uint(intvalue));
    term_puts("\nas float: ");
    value = (double) intvalue;
    term_puts("\n  bytes: ");
    print_double_bytes(value);
    term_puts("\n  float string: ");
    fc_ftoa(buffer, value);
    term_puts(buffer);
    term_puts("\n");
  }

  return 0;
}
