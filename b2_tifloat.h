#ifndef _TI_FLOAT_H
#define _TI_FLOAT_H

int ti_floatToInt(char* bytes);

#include "banking.h"

DECLARE_BANKED(ti_floatToInt, BANK(2), int, bk_ti_floatToInt, (char* bytes), (bytes))

#endif
