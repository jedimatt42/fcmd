#include "speech.h"


typedef void (*read_func)();

// READ_WITH_DELAY() will populate SPEECH_BYTE_BOX
#define READ_WITH_DELAY ((read_func)(SAFE_READ_PAD+2))

// code in scratchpad will read a byte to this address with appropriate
// 'hands-off' period following
#define SPEECH_BYTE_BOX *((volatile unsigned char*)(SAFE_READ_PAD))

// This will be copied to scratch pad for execution at SAFE_READ_PAD + 2
// it is 12 bytes long (gcc will insert the return)
void safe_read() {
  __asm__(
    "MOVB @>9000,@%0\n\t"
    "NOP\n\t"
    "NOP\n\t"
    : : "i" (SAFE_READ_PAD)
  );
}

void delay_asm_12() {
  __asm__(
    "NOP\n\t"
    "NOP"
  );
}

void delay_asm_42() {
  __asm__(
    "\tLI r12,10\n"
    "loop%=\n\t"
    "DEC r12\n\t"
    "JNE loop%="
    : : : "r12"
  );
}

void copy_safe_read() {
  int* source = (int*) safe_read;
  int* target = (int*) (SAFE_READ_PAD+2);
  int len = 12; // bytes
  while(len) {
    *target++ = *source++;
    len -= 2;
  }
}

void load_speech_addr(int phrase_addr) {
  SPCHWT = SPCH_CMD_ADDR | (char)(phrase_addr & 0x000F);
  SPCHWT = SPCH_CMD_ADDR | (char)((phrase_addr >> 4) & 0x000F);
  SPCHWT = SPCH_CMD_ADDR | (char)((phrase_addr >> 8) & 0x000F);
  SPCHWT = SPCH_CMD_ADDR | (char)((phrase_addr >> 12) & 0x000F);

  SPCHWT = SPCH_CMD_ADDR; // end addr
  delay_asm_42(); // I have observed on real hardware, without the delay the speech chip goes a little crazy.
}

void say_vocab(int phrase_addr) {
  load_speech_addr(phrase_addr);
  SPCHWT = SPCH_CMD_VOCAB; // say the phrase
  delay_asm_12();
}

void say_data(const char* addr, int len) {
  SPCHWT = SPCH_CMD_EXT; // say loose data in CPU RAM
  delay_asm_12();
  // Load upto the first 16 bytes
  int i = 16;
  while(i > 0 && len > 0) {
    SPCHWT = *addr++;
    len--;
    i--;
  }
  // Next check for buffer low, and add upto 8 bytes at a time
  while(len > 0) {
    SPEECH_BYTE_BOX = 0;
    while (SPEECH_BYTE_BOX & SPCH_STATUS_LOW == 0) {
      READ_WITH_DELAY();
    }
    // there is room for at least 8 bytes in the FIFO, so send upto 8
    i = 8;
    while(i > 0 && len > 0) {
      SPCHWT = *addr++;
      len--;
      i--;
    }
  }
}

void speech_reset() {
  copy_safe_read();
  SPCHWT = SPCH_CMD_RESET;
  READ_WITH_DELAY(); // EA manual says this is necessary
}

int detect_speech() {
  speech_reset();
  load_speech_addr(0);
  SPCHWT = 0x10;
  delay_asm_12();
  READ_WITH_DELAY();
  return SPEECH_BYTE_BOX == 0xAA;
}

void speech_wait() {
  delay_asm_42();
  delay_asm_12();
  SPEECH_BYTE_BOX = SPCH_STATUS_TALK;
  while (SPEECH_BYTE_BOX & SPCH_STATUS_TALK) {
    READ_WITH_DELAY();
  }
}
