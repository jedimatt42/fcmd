#ifndef _DROPDOWN_H
#define _DROPDOWN_H 1

#include "banking.h"

void ui_drop_down(int linecount);

DECLARE_BANKED_VOID(ui_drop_down, BANK(13), bk_dropDown, (int linecount), (linecount))

#endif