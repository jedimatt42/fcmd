#ifndef _FG99_H
#define _FG99_H 1

#include "banking.h"

extern void fg99(char *msg, int gpl_addr);

DECLARE_BANKED_VOID(fg99, BANK(1), bk_fg99, (char* msg, int gpl_addr), (msg, gpl_addr))

#endif