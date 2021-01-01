#ifndef _RESET_SAMS_H
#define _RESET_SAMS_H 1

#include "banking.h"

void resetSams();

DECLARE_BANKED_VOID(resetSams, BANK(9), bk_resetSams, (), ())


#endif