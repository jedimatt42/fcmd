#ifndef _BANKING_H
#define _BANKING_H 1

#include "banks.h"

extern volatile int* tramp_data;

extern void* trampoline();

/* -- Thanks PeteE! - suggested alternative by PeteE
 * realname - the function you want to be able to call
 * bank - the bank switch address for this function
 * return_type - the type of value returned by the function
 * banked_name - the new name
 * param_types - the parenthesized parameter signature list
 * param_list - the parameter name list
 *
 * NOTE: character (8bit) parameters do not return or pass correctly, use int.
 *
 * Use DECLARE_BANKED in your header files to create
 * convenient banking calls with the same signatures
 * as the original functions.
 *
 * example:  given function in BANK(1), int sum(int a, int b)
 *
 *   DECLARE_BANKED(sum, BANK(1), int, far_sum, (int a, int b), (a, b))
 *
 * generates a static inline function:
 *
 *   static inline int far_sum(int a, int b);
 *
 * Functions that return an 8bit value (char) must use DECLARE_BANKED_CHAR
 * instead. This is required to prevent incorrect manipulation of the
 * return value.
 *
 * Functions with a void return type use a different macro. Usage is the
 * same as the earlier two, but you do not specify the return_type parameter.
 * Given function such as void cputc(const int c):
 *
 *   DECLARE_BANKED_VOID(cputc, BANK(1), far_cputc, (const int c), (c))
 *
 * generates a static inline function:
 *
 *   static inline void far_cputc(const int c);
 *
 * Assumes MYBANK is defined as the callers bank switch address
 *
 * Trampoline code abuses R12, and R0
 */

#define DECLARE_BANKED(realname, bank, return_type, banked_name, param_types, param_list) \
__attribute__ ((gnu_inline, always_inline)) \
static inline return_type banked_name param_types { \
  static const int foo[]={(int)MYBANK, (int)realname, (int)bank}; \
  tramp_data = (int*) foo; \
  return (return_type) trampoline param_list; \
}

#define DECLARE_BANKED_VOID(realname, bank, banked_name, param_types, param_list) \
__attribute__ ((gnu_inline, always_inline)) \
static inline void banked_name param_types { \
  static const int foo[]={(int)MYBANK, (int)realname, (int)bank}; \
  tramp_data = (int*) foo; \
  trampoline param_list; \
}


#endif

