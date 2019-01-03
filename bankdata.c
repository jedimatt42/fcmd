#include "banking.h"

unsigned int bank_stack[2*BANK_STACK_SIZE];
unsigned int* bank_return = bank_stack;

int* tramp_data;

int tramp_stash; // used by trampoline

