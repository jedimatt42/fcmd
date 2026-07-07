#ifndef _ED_H
#define _ED_H 1

#include "banking.h"

int commandLineEd(char* cmdline, int limit);

DECLARE_BANKED(commandLineEd, BANK(13), int, bk_commandLineEd, (char* cmdline, int limit), (cmdline, limit))

#endif
