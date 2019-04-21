#ifndef _VARIABLES_H
#define _VARIABLES_H

#include "banking.h"

char* vars_get(char* name);
void vars_set(char* name, char* value);

DECLARE_BANKED(vars_get, BANK_4, int, bk_vars_get, (char* name), (name))
DECLARE_BANKED_VOID(vars_set, BANK_4, bk_vars_set, (char* name, char* value), (name, value))

#endif