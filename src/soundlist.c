#include "banks.h"
#define MYBANK BANK(13)

#include "soundlist.h"
#include "sound.h"
#include "vdp.h"

static const unsigned char *snd_ptr;
static unsigned char   snd_timer;
static unsigned char   snd_active;

void snd_start(const unsigned char *list) {
    snd_ptr    = list;
    snd_timer  = 0;
    snd_active = 1;
}

void snd_tick(void) {
    unsigned char cmd;
    unsigned char cnt;
    unsigned int  addr;

    if (!snd_active) return;

    if (snd_timer > 0) {
        --snd_timer;
        return;
    }

    cmd = *snd_ptr++;

    if (cmd == 0x00 || cmd == 0xFF) {
        /* Toggle: in the console ISR 0xFF swaps data paths.
         * The byte is followed by 2 address bytes (like set-pointer).
         * We ignore the path toggle and just chain to the new address. */

        /* Set pointer: read lo/hi bytes, jump to new address */
        addr  = *snd_ptr++;
        addr |= (unsigned int)(*snd_ptr++) << 8;
        snd_ptr    = (const unsigned char *)addr;
        snd_timer  = 1;
    } else {
        /* Data block: cmd = count, then count data bytes, then delay */
        cnt = cmd;
        while (cnt--) {
            SOUND = *snd_ptr++;
        }
        snd_timer = *snd_ptr++;
        if (snd_timer == 0) snd_active = 0;
    }
}

int snd_playing(void) {
    return snd_active;
}

void snd_stop(void) {
    snd_active = 0;
    MUTE_SOUND();
}

void snd_play(const unsigned char *playlist) {
    snd_start(playlist);
    do {
        VDP_WAIT_VBLANK_CRU;
        snd_tick();
    } while (snd_playing());
    snd_stop();
    VDP_WAIT_VBLANK_CRU;
}
