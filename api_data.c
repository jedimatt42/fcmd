#include "banking.h"
#include "procinfo.h"

void fc_api();
void stramp();
// gcc runtime routines to create a branch table to
void memcpy();

// address of fc_api routine
int api_fc_api = (int)fc_api;

// make this not zero, so it is in the data segment 
int trampdata = !0;

// address of sams trampoline routine
int api_stramp = (int)stramp;

// current process info
struct ProcInfo* procInfoPtr = (struct ProcInfo*) 0xffff;

// Branch table to gcc runtime function always in bank 0
int gcc_memcpy[] = {
  0x0460, (int)memcpy
};

