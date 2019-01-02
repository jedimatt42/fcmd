#ifndef _BANKING_H
#define _BANKING_H 1

/*
Use BANK_CALL macro to:
  1. caller to store their bank, the target bank, and the target address.

  2. Then cast the function trampoline to their original target's signature and call it.
     This should set up the stack and registers for calling the actual function. 

  3. Trampoline will then store the return address, and call the new function.

  4. when that function returns to trampoline, it will restore the bank, and the
     return address, then actually return.
*/


extern volatile int* dest_bank;
extern void* dest_func_addr;
extern unsigned int* bank_return;

extern void* trampoline();

// f=function address, db=function bank address, t=signature of function
//
// examples of t - the signature:
//  void foo(); t = void (*)(void)
//  int  bar(int x); t = int (*)(int)
// 
#define BANK_CALL(f,db,t) \
  __extension__ \
  ((t) ({ dest_func_addr=f; dest_bank=(int*)db; *bank_return=(unsigned int)MYBANK; trampoline;}))

/*
 * suggested alternative by PeteE
 *

#define DECLARE_BANKED(realname, bank, fn_prototype) \
static inline fn_prototype { \
 asm volatile( \
   "li r0,%0  \n\t" \
   "mov r0,@dest_func_addr \n\t" \
   "li r0,%1  \n\t" \
   "mov r0,@dest_bank \n\t" \
   "li r0,%2  \n\t" \
   "mov r0,@caller_bank \n\t" \
   "bl @trampoline \n\t" \
   : \
   : "i"(realname),"i"(bank),"i"(MYBANK) \
   : "r0" \
 ); \
}

*/

#endif

