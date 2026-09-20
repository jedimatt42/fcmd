#include "fctest.h"

/*
 * I10 - built-in terminal sounds.
 *
 * Covers: audio_beep, audio_honk.
 *
 * Both functions are void and block until the tone finishes (they drive the
 * sound list player a frame at a time), so this is an observation-only test.
 */

int main(char* args) {
    (void)args;

    term_puts("AUDIO test\n");

    term_puts("[OBSERVE] a short high beep should sound now...\n");
    audio_beep();
    FC_OBSERVE("beep finished; you should have heard one short high tone");

    term_puts("[OBSERVE] a lower honk should sound now...\n");
    audio_honk();
    FC_OBSERVE("honk finished; you should have heard one lower tone");

    return fc_summary();
}
