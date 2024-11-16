/* double.c
   This file includes double math functions that the GCC compiler requires.
   06/23/2023 mrvan initial version
   08/14/2023 mrvan removed methods not associated with the FAC and ARG to separate files
   03/22/2024 jedimatt42 eliminate data segment allocations for FAC and ARG pointers
*/

#define FAC 0x834A
#define ARG 0x835C
#define FAC_PTR ((volatile double*)FAC)
#define ARG_PTR ((volatile double*)ARG)

#define FADD_AUGEND FAC_PTR
#define FADD_ADDEND ARG_PTR
#define FADD_SUM    FAC_PTR

#define FSUB_MINUEND    ARG_PTR
#define FSUB_SUBTRAHEND FAC_PTR
#define FSUB_DIFFERENCE FAC_PTR

#define FMUL_MULTIPLICAND ARG_PTR
#define FMUL_MULTIPLIER   FAC_PTR
#define FMUL_PRODUCT      FAC_PTR

#define FDIV_DIVIDEND FAC_PTR
#define FDIV_DIVISOR  ARG_PTR
#define FDIV_QUOTIENT FAC_PTR


// double addition -- method normally included in GCC lib
double __adddf3(double a, double b) {

   // use the ti99's ROM method

   // adds a and b
   *FADD_AUGEND = a;
   *FADD_ADDEND = b;

  __asm__(
    "lwpi >83E0\n\t"
    "bl @>0D80\n\t"
    "lwpi >8300\n\t"
  );

  return *FADD_SUM;
}

// double subtraction -- method normally included in GCC lib
double __subdf3(double a, double b) {

   // use the ti99's ROM method

   // subtract b from a
   *FSUB_MINUEND    = a;
   *FSUB_SUBTRAHEND = b;

  __asm__(
    "lwpi >83E0\n\t"
    "bl @>0D7C\n\t"
    "lwpi >8300\n\t"
  );

  return *FSUB_DIFFERENCE;
}

// double multiplication -- method normally included in GCC lib
double __muldf3(double a, double b) {

   // use the ti99's ROM method

   // multiply a and b
   *FMUL_MULTIPLICAND = a;
   *FMUL_MULTIPLIER   = b;

  __asm__(
    "lwpi >83E0\n\t"
    "bl @>0E88\n\t"
    "lwpi >8300\n\t"
  );

  return *FMUL_PRODUCT;
}

// double division -- method normally included in GCC lib
double __divdf3(double a, double b) {

   // use the ti99's ROM method

   // divide b by a
   *FDIV_DIVIDEND = b;
   *FDIV_DIVISOR  = a;

  __asm__(
    "lwpi >83E0\n\t"
    "bl @>0FF4\n\t"
    "lwpi >8300\n\t"
  );

  return *FDIV_QUOTIENT;
}

typedef struct {
    char exponent;   // 1 byte exponent
    char mantissa[7]; // 7 bytes mantissa
} Radix100Float;

double __floatsidf(int a) {
    Radix100Float r100;
    int is_negative = 0;

    // Handle zero case
    if (a == 0) {
        r100.exponent = 0x00;  // exponent = 0
        for (int i = 0; i < 7; i++) {
            r100.mantissa[i] = 0x00;  // mantissa = 0
        }
        return *(double*)&r100;  // Cast the structure to double and return
    }

    // Handle negative numbers
    if (a < 0) {
        is_negative = 1;
        a = -a;  // Convert to positive for processing
    }

    // Step 1: Determine the exponent
    int exponent = 0;
    int temp = a;
    while (temp >= 100) {
        temp /= 100;
        exponent++;
    }

    // Step 2: Adjust for bias
    r100.exponent = 0x40 + exponent;  // Add bias of 64 (0x40)

    // Step 3: Extract mantissa (radix-100 digits)
    for (int i = exponent; i >= 0; i--) {
        r100.mantissa[i] = a % 100;  // Get last two digits
        a /= 100;
    }
    // zero out rest of mantissa
    for (int i = exponent + 1; i < 7; i++) {
      r100.mantissa[i] = 0;
    }

    // Step 4: Apply sign if negative
    if (is_negative) {
        r100.exponent = ~r100.exponent + 1;  // Two's complement of exponent
        r100.mantissa[0] = ~r100.mantissa[0] + 1;  // Two's complement of first byte of mantissa
    }

    // Return the encoded radix-100 floating-point value by casting the structure to double
    return *(double*)&r100;
}

int __fixdfsi(double a) {
//     "bl @>12b8\n\t"
  *FAC_PTR = a;

  __asm__(
    "lwpi >83E0\n\t"
    "bl @>12b8\n\t"
    "lwpi >8300\n\t"
  );

  return *(int*)FAC_PTR;
}
