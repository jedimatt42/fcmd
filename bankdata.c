#include "banking.h"

volatile int* dest_bank;
void* dest_func_addr;

unsigned int bank_stack[2*BANK_STACK_SIZE];
unsigned int* bank_return = bank_stack;

unsigned int tramp_stash; // used by trampoline

