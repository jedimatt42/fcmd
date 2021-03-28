#include "fc_api.h"

#define SPCHRD    *((volatile unsigned char*)0x9000)
#define SPCHWT		*((volatile unsigned char*)0x9400)

#define SPCH_CMD_RESET 0x70
#define SPCH_CMD_EXT 0x60
#define SPCH_CMD_VOCAB 0x50
#define SPCH_CMD_ADDR 0x40
#define SPCH_CMD_READ 0x10

#define SPEECH_HELLO 0x351A

const char CUSTOM_PHRASE[] = {
  166, 209,
  198,37,104,82,151,
  206,91,138,224,232,
  116,186,18,85,130,
  204,247,169,124,180,
  116,239,185,183,184,
  197,45,20,32,131,
  7,7,90,29,179,
  6,90,206,91,77,
  136,166,108,126,167,
  181,81,155,177,233,
  230,0,4,170,236,
  1,11,0,170,100,
  53,247,66,175,185,
  104,185,26,150,25,
  208,101,228,106,86,
  121,192,234,147,57,
  95,83,228,141,111,
  118,139,83,151,106,
  102,156,181,251,216,
  167,58,135,185,84,
  49,209,106,4,0,
  6,200,54,194,0,
  59,176,192,3,0,
  0
};

typedef void (*read_func)();

// READ_WITH_DELAY() will populate SPEECH_BYTE_BOX
#define READ_WITH_DELAY ((read_func)0x8322)

// code in scratchpad will read a byte to this address with appropriate
// 'hands-off' period following
#define SPEECH_BYTE_BOX *((volatile unsigned char*)0x8320)

// This will be copied to scratch pad for execution >8322
// it is 12 bytes long (gcc will insert the return)
void safe_read() {
  __asm__(
    "MOVB @>9000,@0x8320\n\t"
    "NOP\n\t"
    "NOP\n\t"
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
  int* target = (int*) 0x8322;
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
  while(len > 0) {
    SPCHWT = *addr++; // If buffer is full, speech will use READY to hold CPU
    len--;
  }
}

void speech_reset() {
  SPCHWT = SPCH_CMD_RESET;
  READ_WITH_DELAY(); // voodoo
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
  SPEECH_BYTE_BOX = 0x80;
  while (SPEECH_BYTE_BOX & 0x80) {
    READ_WITH_DELAY();
  }
}

int main(char* args) {
  copy_safe_read();

  if (!detect_speech()) {
    fc_tputs("No speech synthesizer detected :(\n");
    return 0;
  }

  say_vocab(SPEECH_HELLO);
  speech_wait();

  speech_reset();
  say_data(CUSTOM_PHRASE, 118);
  speech_wait();

  return 0;
}
