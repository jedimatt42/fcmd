#ifndef _HONK_H
#define _HONK_H 1

#include "banking.h"

void audio_honk();
void audio_beep();

DECLARE_BANKED_VOID(audio_honk, BANK(13), bk_honk, (), ())
DECLARE_BANKED_VOID(audio_beep, BANK(13), bk_beep, (), ())

#endif