#ifndef _BANKING_H
#define _BANKING_H 1

extern volatile unsigned int* dest_bank;
extern void* dest_func_addr;
extern volatile unsigned int* caller_bank;
extern void* caller_return_addr;

// untyped function pointer to the trampoline
extern void* tramp_func;

#define FAR_CALL(f,db,t) dest_func_addr=f, dest_bank=db, caller_bank=MYBANK, ((t)tramp_func)

#endif

