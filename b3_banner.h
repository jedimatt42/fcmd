#ifndef _BANNER_H
#define _BANNER_H

#include "banking.h"

void banner();

DECLARE_BANKED_VOID(banner, BANK(3), bk_banner, (), ())

#endif