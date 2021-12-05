#include "banking.h"

void fc_api();
void stramp();

// address of fc_api routine
int api_fc_api = (int)fc_api;

// make this not zero, so it is in the data segment 
int trampdata = !0;

// address of sams trampoline routine
int api_stramp = (int)stramp;

