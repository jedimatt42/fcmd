#ifndef _SOUNDLIST_H
#define _SOUNDLIST_H 1

#include "banking.h"

void snd_start(const unsigned char *list);
void snd_tick(void);
int  snd_playing(void);
void snd_stop(void);
void snd_play(const unsigned char *list);

DECLARE_BANKED_VOID(snd_start, BANK(13), bk_snd_start, (const unsigned char *list), (list))
DECLARE_BANKED_VOID(snd_tick,  BANK(13), bk_snd_tick,  (), ())
DECLARE_BANKED(snd_playing,     BANK(13), int, bk_snd_playing, (), ())
DECLARE_BANKED_VOID(snd_stop,  BANK(13), bk_snd_stop,  (), ())
DECLARE_BANKED_VOID(snd_play,  BANK(13), bk_snd_play,  (const unsigned char *list), (list))

#endif
