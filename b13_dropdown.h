#ifndef _DROPDOWN_H
#define _DROPDOWN_H 1

#include "banking.h"

void dropDown(int linecount);

DECLARE_BANKED_VOID(dropDown, BANK(13), bk_dropDown, (int linecount), (linecount))

#endif