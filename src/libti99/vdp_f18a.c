#include "banks.h"
#define MYBANK BANK(8)

#include "vdp.h"

void lock_f18a() {
    VDP_SET_REGISTER(0x32, 0x80);
    VDP_SET_REGISTER(0x02, 0x00);
}

void unlock_f18a() {
    // Clear the unlock bit first. Two >1C writes only unlock from a
    // known-locked state; if already unlocked, the first >1C re-locks it.
    VDP_SET_REGISTER(0x39, 0x00);
    VDP_SET_REGISTER(0x39, 0x1c);  // Write once
    VDP_SET_REGISTER(0x39, 0x1c);  // Write twice, unlock
}

