#ifndef _EXEC_H
#define _EXEC_H 1

#include "banking.h"

void exec(const char* command);

DECLARE_BANKED_VOID(exec, BANK(0), bk_exec, (const char* command), (command))

#endif