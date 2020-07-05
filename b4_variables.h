#ifndef _VARIABLES_H
#define _VARIABLES_H

#include "banking.h"

// Lets keep these odd numbers so null terminate makes even
#define MAX_VAR_NAME 11
#define MAX_VAR_VAL 81

char* vars_get(char* name);
void vars_set(char* name, char* value);
void printVars();

DECLARE_BANKED(vars_get, BANK_4, char*, bk_vars_get, (char* name), (name))
DECLARE_BANKED_VOID(vars_set, BANK_4, bk_vars_set, (char* name, char* value), (name, value))

#endif