#include "banking.h"

volatile int* dest_bank;
void* dest_func_addr;
volatile int* caller_bank;
void* caller_return_addr;

int sample = 0xCAFE;

