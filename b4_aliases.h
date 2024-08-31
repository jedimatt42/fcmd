#ifndef _ALIASES_H
#define _ALIASES_H

#include "banking.h"

// Lets keep these odd numbers so null terminate makes even
#define MAX_ALIAS_NAME 11
#define MAX_ALIAS_VAL 81

char* alias_get(char* name);
void alias_set(char* name, char* value);

DECLARE_BANKED(alias_get, BANK(4), char*, bk_alias_get, (char* name), (name))
DECLARE_BANKED_VOID(alias_set, BANK(4), bk_alias_set, (char* name, char* value), (name, value))

#endif