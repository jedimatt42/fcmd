#include "fctest.h"
#include <vocab.h>

/*
 * I23 - speech synthesizer.
 *
 * Covers: speech_detect, speech_reset, speech_say_vocab, speech_start,
 *         speech_continue, speech_wait.
 *
 * Skips when no synthesizer is detected. The vocabulary phrase is audible and
 * observed. speech_start / speech_continue are checked for buffer accounting
 * using a short dummy buffer, then the synthesizer is reset.
 */

static void test_speech(void) {
    struct LpcPlaybackCtx ctx;
    char lpc[20];
    int detected;
    int guard;

    detected = speech_detect();
    if (!detected) {
        FC_SKIP("no speech synthesizer detected");
        return;
    }
    FC_CHECK(detected != 0);

    speech_reset();

    term_puts("[OBSERVE] the synthesizer should say 'hello' now\n");
    speech_say_vocab(VOCAB_HELLO);
    speech_wait();
    FC_OBSERVE("the vocabulary phrase finished");

    /* speech_start sends up to 16 bytes; speech_continue feeds more */
    str_set(lpc, 0, sizeof(lpc));
    ctx.addr = lpc;
    ctx.remaining = (int)sizeof(lpc);

    speech_start(&ctx);
    FC_CHECK(ctx.remaining <= (int)sizeof(lpc));
    FC_CHECK(ctx.addr == lpc + ((int)sizeof(lpc) - ctx.remaining));

    guard = 2000;
    while (ctx.remaining > 0 && guard-- > 0) {
        speech_continue(&ctx);
    }
    FC_CHECK_EQ(ctx.remaining, 0);

    speech_reset();
    FC_OBSERVE("synthesizer reset");
}

int main(char* args) {
    (void)args;

    term_puts("SPEECHTEST test\n");

    test_speech();

    return fc_summary();
}
