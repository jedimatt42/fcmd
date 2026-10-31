#ifndef _VARIABLES_H
#define _VARIABLES_H

#include "banking.h"

// Lets keep these odd numbers so null terminate makes even
#define MAX_VAR_NAME 11
#define MAX_VAR_VAL 81

// Copy the value of variable `name` into `buf`. `limit` is the size of `buf`,
// including the trailing NUL. Returns 1 when found and copied, 0 otherwise.
// The name is not modified. No pointer into the FC dictionary is returned.
int var_get(const char* name, char* buf, int limit);
void var_set(const char* name, const char* value);
void printVars();

DECLARE_BANKED(var_get, BANK(4), int, bk_vars_get, (const char* name, char* buf, int limit), (name, buf, limit))
DECLARE_BANKED_VOID(var_set, BANK(4), bk_vars_set, (const char* name, const char* value), (name, value))

#endif