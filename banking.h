#ifndef _BANKING_H
#define _BANKING_H 1

#include "banks.h"

extern volatile int* tramp_data;

extern void* trampoline();

/* -- Thanks PeteE! - suggested alternative by PeteE
 * realname - the function you want to be able to call
 * bank - the bank switch address for this function
 * fn_prototype - the new name and signature to declare
 * trampcall - how the trampoline should be called with
 *             argument list and return statement.
 *
 * Use DECLARE_BANKED in your header files to create
 * convenient banking calls with the same signatures
 * as the original functions. 
 * 
 * example:  given function in BANK_1, int sum(int a, int b)
 *
 *   DECLARE_BANKED(sum, BANK_1, int far_sum(int a, int b), return trampoline(a,b))
 *
 * generates a static inline function:
 *
 *   static inline int far_sum(int a, int b);
 *
 * Assumes MYBANK is defined as the callers bank switch address
 * 
 * Trampoline code abuses R12, and R0
 */
#define DECLARE_BANKED(realname, bank, fn_prototype, trampcall) \
static inline fn_prototype { \
  static const int foo[]={MYBANK, (int)realname, bank}; \
  tramp_data = (int*) foo; \
  trampcall; \
}

#endif

