
/*
  1. caller to store their bank, the target bank, and the target address.

  2. Then cast the function trampoline to their original target's signature and call it.
     This should set up the stack and registers for calling the actual function. 

  3. Trampoline will then store the return address, and call the new function.

  4. when that function returns to trampoline, it will restore the bank, and the
     return address, then actually return.
*/

volatile unsigned int* dest_bank;
void* dest_func_addr;
volatile unsigned int* caller_bank;
void* caller_return_addr;

void trampoline(void);

void* tramp_func = trampoline;

void trampoline(void) {
  __asm__("mov r11,@caller_return_addr");
  *dest_bank = 0; // trigger switch to destination bank.

  *caller_bank = 0; // switch back to caller bank.
  __asm__("mov @caller_return_addr,r11");
}

