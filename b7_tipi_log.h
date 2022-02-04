#ifndef _tipi_mouse
#define _tipi_mouse 1

#include "banking.h"

void tipi_log(char* msg);

DECLARE_BANKED_VOID(tipi_log, BANK(7), bk_tipi_log, (char* msg), (msg))

#endif

