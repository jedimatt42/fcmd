#ifndef _HONK_H
#define _HONK_H 1

#include "banking.h"

void honk();
void beep();

DECLARE_BANKED_VOID(honk, BANK(13), bk_honk, (), ())
DECLARE_BANKED_VOID(beep, BANK(13), bk_beep, (), ())

#endif