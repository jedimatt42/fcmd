#ifndef _ALIASES_H
#define _ALIASES_H

#include "banking.h"

// Lets keep these odd numbers so null terminate makes even
#define MAX_ALIAS_NAME 11
#define MAX_ALIAS_VAL 81

// Copy the value of alias `name` into `buf`. `limit` is the size of `buf`,
// including the trailing NUL. Returns 1 when found and copied, 0 otherwise.
// The name is not modified. No pointer into the FC dictionary is returned.
int alias_get(const char* name, char* buf, int limit);
void alias_set(const char* name, const char* value);

DECLARE_BANKED(alias_get, BANK(4), int, bk_alias_get, (const char* name, char* buf, int limit), (name, buf, limit))
DECLARE_BANKED_VOID(alias_set, BANK(4), bk_alias_set, (const char* name, const char* value), (name, value))

#endif