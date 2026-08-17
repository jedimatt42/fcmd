#ifndef _RUN_EXT_H
#define _RUN_EXT_H 1

#include "banking.h"

int runExecutable(const char* ext);

DECLARE_BANKED(runExecutable, BANK(0), int, bk_runExecutable, (const char* ext), (ext))

/* Terminate the current executable without unwinding its call stack. */
void fc_exit_prepare(unsigned int resume);
void fc_exit(int status);
DECLARE_BANKED_VOID(fc_exit, BANK(0), bk_fc_exit, (int status), (status))

#endif
