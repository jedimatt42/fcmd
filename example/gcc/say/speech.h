#ifndef _SPEECH_H
#define _SPEECH_H 1

#define SPCHRD    *((volatile unsigned char*)0x9000)
#define SPCHWT		*((volatile unsigned char*)0x9400)

#define SPCH_CMD_RESET 0x70
#define SPCH_CMD_EXT 0x60
#define SPCH_CMD_VOCAB 0x50
#define SPCH_CMD_ADDR 0x40
#define SPCH_CMD_READ 0x10

// Define a default location in scratch pad for the safe read routine and
// status byte mailbox. Requires 14 bytes.
#ifndef SAFE_READ_PAD
#define SAFE_READ_PAD 0x8320
#endif


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
 * With an LPC code loaded into CPU RAM at addr, say_data sends the say_external command and transmits the LCP into the synthesizer's FIFO.
 */
void say_data(const char* addr, int len);

/*
 * Wait for speech to finish producing the current phrase.
 */
void speech_wait();

#endif
