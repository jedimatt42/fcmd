#ifndef _EXEC_H
#define _EXEC_H 1

#include "types.h"
#include "banking.h"

int16_t exec(char* command);

DECLARE_BANKED(exec, BANK(0), int16_t, bk_exec, (char* command), (command))

#endif
