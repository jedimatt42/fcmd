#include "banking.h"
#include "procinfo.h"

#define BRANCH 0x0460

void fc_api();
void stramp();

// fake function declarations for gcc runtime routines to create a branch table to
void memcpy();
void memset();
void __adddf3();
void __subdf3();
void __muldf3();
void __divdf3();
void d_compare();
void __eqdf2();

// address of fc_api routine
int api_fc_api = (int)fc_api;

// make this not zero, so it is in the data segment 
int trampdata = !0;

// address of sams trampoline routine
int api_stramp = (int)stramp;

// current process info
struct ProcInfo* procInfoPtr = (struct ProcInfo*) 0xffff;

// Branch table to gcc runtime function always in bank 0
int gcc_memcpy[] = { BRANCH, (int)memcpy };
int gcc_memset[] = { BRANCH, (int)memset };
int gcc_adddf3[] = { BRANCH, (int)__adddf3 };
int gcc_subdf3[] = { BRANCH, (int)__subdf3 };
int gcc_muldf3[] = { BRANCH, (int)__muldf3 };
int gcc_divdf3[] = { BRANCH, (int)__divdf3 };
int gcc_d_compare[] = { BRANCH, (int)d_compare };
int gcc_eqdf2[] = { BRANCH, (int)__eqdf2 };

