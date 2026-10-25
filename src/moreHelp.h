#ifndef _B14_HELP_H
#define _B14_HELP_H 1

#include "banking.h"

void b14Help(char* tok);
int moreHelp2(char* tok);

DECLARE_BANKED_VOID(b14Help, BANK(14), bk_b14Help, (char* tok), (tok))
DECLARE_BANKED(moreHelp2, BANK(6), int, bk_moreHelp2, (char* tok), (tok))

#endif
