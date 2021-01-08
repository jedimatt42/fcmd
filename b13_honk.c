#include "banks.h"
#define MYBANK BANK(13)

#include "b13_honk.h"
#include "sound.h"
#include "vdp.h"

static void play(char* playlist, unsigned int count);

const char pl_honk[] = {
    0x06, 0xbf, 0xdf,
    0xff, 0x80, 0x20,
    0x90, 0x0a,
    0x01, 0x9f, 0x00
};

const char pl_beep[] = {
    0x06, 0xbf, 0xdf,
    0xff, 0x80, 0x05,
    0x92, 0x0a,
    0x01, 0x9f, 0x00
};

void honk() {
    char* pSrc = (char*)pl_honk;
    unsigned int cnt = sizeof(pl_honk);
    play(pSrc, cnt);
}

void beep() {
    char* pSrc = (char*)pl_beep;
    unsigned int cnt = sizeof(pl_beep);
    play(pSrc, cnt);
}

static void play(char* playlist, unsigned int count) {
    VDP_SET_ADDRESS_WRITE(0x3E00);
    while (count--) {
        VDPWD = *(playlist++);
    }
    SET_SOUND_PTR(0x3E00);
    SET_SOUND_VDP();
    START_SOUND();
    while (SOUND_CNT != 0) {
        VDP_INT_POLL;
    }
    MUTE_SOUND();
    VDP_INT_POLL;
}