#ifndef _SPEECH_H
#define _SPEECH_H 1

#define SPCHRD    *((volatile unsigned char*)0x9000)
#define SPCHWT		*((volatile unsigned char*)0x9400)

#define SPCH_CMD_RESET 0x70
#define SPCH_CMD_EXT 0x60
#define SPCH_CMD_VOCAB 0x50
#define SPCH_CMD_ADDR 0x40
#define SPCH_CMD_READ 0x10

#define SPCH_STATUS_TALK 0x80
#define SPCH_STATUS_LOW 0x40

// Define a default location in scratch pad for the safe read routine and
// status byte mailbox. Requires 14 bytes.
#ifndef SAFE_READ_PAD
#define SAFE_READ_PAD 0x8320
#endif


struct __attribute__((__packed__)) LpcPlaybackCtx {
    char* addr;
    int remaining;
};


/*
 * Issue the reset command to the speech synthesizer.
 * (this is automatically included in a detect_speech call)
 */
void speech_reset();

/*
 * Return 1 if speech synthesizer is attached to the 4A.
 * It sends the reset cmd, then a read of PHROM address 0x0000.
 */
int detect_speech();

/*
 * Say the vocabulary phrase in the speech ROM (PHROM) at the specified address.
 * See EA manual for addresses of each built in phrase.
 */
void say_vocab(int phrase_addr);

/*
 * With an LPC code loaded into CPU RAM at addr, say_data sends the say_external command and transmits the entire LCP into the synthesizer's FIFO.
 */
void say_data(const char* addr, int len);

/*
 * Given a LpcPlaybackCtx with a pointer to the LPC data, and the remaining bytes to send, sends the command code and upto the first 16 bytes of the
 * LPC data. ctx.remaining and ctx.addr are modified to be used by subsequent speech_continue calls.
 */
void speech_start(struct LpcPlaybackCtx* ctx);

/*
 * Continue feeding speech data to the synthesizer, upto 8 bytes per call. ctx is modified.
 */
void speech_continue(struct LpcPlaybackCtx* ctx);

/*
 * Wait for speech to finish producing the current phrase.
 */
void speech_wait();

#endif
