#ifndef _LIBTORAM_H
#define _LIBTORAM_H

void libtoram();

#include "banking.h"

DECLARE_BANKED_VOID(libtoram, BANK(1), bk_libtoram, (), ())

#endif

