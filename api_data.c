#include "banking.h"
#include "procinfo.h"

#define BRANCH 0x0460

void fc_api();
void stramp();
void fc_cc_deref_handle();

// fake function declarations for gcc runtime routines to create a branch table to
void memcpy();
void memset();
void __adddf3();
void __subdf3();
void __muldf3();
void __divdf3();
void d_compare();
void __eqdf2();
void __floatsidf();
void __fixdfsi();

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
int gcc_floatsidf[] = { BRANCH, (int)__floatsidf };
int gcc_fixdfsi[] = { BRANCH, (int)__fixdfsi };

// fc-cc runtime helpers — follows the GCC branch table at 0x2030+
// 2-slot heap page ledger for __deref_handle: { last_used, slot0_page_id, slot1_page_id }
// Layout: ledger at 0x2030-0x2034, funcptr at 0x2036 (declaration order, GCC places sequentially)
// Pre-initialized to 0xFCFC to force .data placement (GCC would put zeros in .bss).
// Zeroed by early startup before any fc-cc program runs.
int fc_cc_ledger[3] = { 0xFCFC, 0xFCFC, 0xFCFC };
// Address of the __deref_handle routine in cartridge ROM bank 0
int fc_cc_deref_handle_addr = (int)fc_cc_deref_handle;

