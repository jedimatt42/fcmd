#ifndef _EXEC_H
#define _EXEC_H 1

#include "banking.h"

int exec_cmd(char* command);

DECLARE_BANKED(exec_cmd, BANK(0), int, bk_exec, (char* command), (command))

#endif
