#ifndef _EXEC_H
#define _EXEC_H 1

#include "banking.h"

int exec(char* command);

DECLARE_BANKED(exec, BANK(0), int, bk_exec, (char* command), (command))

#endif