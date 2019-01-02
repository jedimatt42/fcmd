#ifndef _BANKING_H
#define _BANKING_H 1

#include "banks.h"

extern volatile int* dest_bank;
extern void* dest_func_addr;
extern unsigned int* bank_return;
extern unsigned int tramp_stash;

extern void* trampoline();

/* -- Thanks PeteE! - suggested alternative by PeteE
 * realname - the function you want to be able to call
 * bank - the bank switch address for this function
 * fn_prototype - the new name and signature to declare
 *
 * Use DECLARE_BANKED in your header files to create
 * convenient banking calls with the same signatures
 * as the original functions. 
 * 
 * example:  given function in BANK_1, int sum(int a, int b)
 *
 *   DECLARE_BANKED(sum, BANK_1, int far_sum(int a, int b))
 *
 * generates a static inline function:
 *
 *   static inline int far_sum(int a, int b);
 *
 * Assumes MYBANK is defined as the callers bank switch address
 */
#define DECLARE_BANKED(realname, bank, fn_prototype, trampcall) \
static inline fn_prototype { \
 __asm__ volatile( \
   "li r0,%0  \n\t" \
   "mov r0,@dest_func_addr \n\t" \
   "li r0,%1  \n\t" \
   "mov r0,@dest_bank \n\t" \
   "li r0,%2  \n\t" \
   "mov r0,@tramp_stash \n\t" \
   "mov @bank_return,r0 \n\t" \
   "mov @tramp_stash,*r0 \n\t" \
   : \
   : "i"(realname),"i"(bank),"i"(MYBANK) \
   : "r0" \
 ); \
 \
 trampcall; \
}

#endif

