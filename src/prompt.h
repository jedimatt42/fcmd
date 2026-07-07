#ifndef _PROMPT_H
#define _PROMPT_H 1

#include "banking.h"

void print_prompt(char* pattern);

DECLARE_BANKED_VOID(print_prompt, BANK(5), bk_print_prompt, (char* pattern), (pattern))

#endif