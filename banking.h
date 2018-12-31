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
extern volatile int* caller_bank;
extern void* caller_return_addr;

// untyped function pointer to the trampoline
extern void* tramp_func;

// f=function address, db=function bank address, t=signature of function, a=argument list 
#define BANK_CALL(f,db,t) \
  __extension__ \
  ((t) ({ dest_func_addr=f; dest_bank=(int*)db; caller_bank=(int*)MYBANK; tramp_func;}))

#endif

