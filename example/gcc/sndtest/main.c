#include "fctest.h"

/*
 * I11 - sound list player.
 *
 * Covers: snd_start, snd_tick, snd_playing, snd_stop, snd_play.
 *
 * Sound list format (see internal-docs/soundplayer.md):
 *   - byte 1..254: data block; that many bytes go to the sound chip, then one
 *     delay byte. A delay of 0 ends playback.
 *   - byte 0x00 or 0xFF: set pointer; two address bytes follow.
 *
 * The AUTO lists below write only "volume off" bytes (0x9f / 0xbf), so they
 * are silent. probe_tone is the audible one.
 */

/* never terminates on its own; used for start/stop state checks */
static const unsigned char endless[] = {
    0x01, 0x9f, 0x32
};

/* one data block with a delay of 0: ends on the first tick */
static const unsigned char one_shot[] = {
    0x01, 0x9f, 0x00
};

/* first block sets a 3-tick delay, second block terminates */
static const unsigned char delayed[] = {
    0x01, 0x9f, 0x03,
    0x01, 0xbf, 0x00
};

/* audible short beep on channel 0 */
static const unsigned char probe_tone[] = {
    0x06, 0xbf, 0xdf, 0xff, 0x80, 0x05, 0x92, 0x0a,
    0x01, 0x9f, 0x00
};

static void test_start_stop(void) {
    snd_start(endless);
    FC_CHECK_EQ(snd_playing(), 1);

    snd_stop();
    FC_CHECK_EQ(snd_playing(), 0);
}

static void test_tick_ends(void) {
    snd_start(one_shot);
    FC_CHECK_EQ(snd_playing(), 1);

    snd_tick();
    FC_CHECK_EQ(snd_playing(), 0);

    /* ticking while inactive is a no-op */
    snd_tick();
    FC_CHECK_EQ(snd_playing(), 0);
}

static void test_tick_delay(void) {
    int i;

    snd_start(delayed);
    FC_CHECK_EQ(snd_playing(), 1);

    /* first tick consumes the first block and arms the 3-tick delay */
    snd_tick();
    FC_CHECK_EQ(snd_playing(), 1);

    /* three ticks burn the delay, the next consumes the terminator */
    for (i = 0; i < 4; i++) {
        snd_tick();
    }
    FC_CHECK_EQ(snd_playing(), 0);
}

static void test_play(void) {
    term_puts("[OBSERVE] a short beep should sound now...\n");
    snd_play(probe_tone);
    FC_CHECK_EQ(snd_playing(), 0);
}

int main(char* args) {
    (void)args;

    term_puts("SNDTEST test\n");

    test_start_stop();
    test_tick_ends();
    test_tick_delay();
    test_play();

    return fc_summary();
}
