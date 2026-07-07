#ifndef _IS_PAL_H
#define _IS_PAL_H 1

#include "banking.h"

int isPal();

DECLARE_BANKED(isPal, BANK(10), int, bk_isPal, (), ())

#endif