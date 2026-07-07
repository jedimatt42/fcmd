#ifndef _VARIABLES_H
#define _VARIABLES_H

#include "banking.h"

// Lets keep these odd numbers so null terminate makes even
#define MAX_VAR_NAME 11
#define MAX_VAR_VAL 81

char* var_get(char* name);
void var_set(char* name, char* value);
void printVars();

DECLARE_BANKED(var_get, BANK(4), char*, bk_vars_get, (char* name), (name))
DECLARE_BANKED_VOID(var_set, BANK(4), bk_vars_set, (char* name, char* value), (name, value))

#endif