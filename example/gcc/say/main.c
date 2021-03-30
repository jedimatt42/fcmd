#include "fc_api.h"
#include "speech.h"

#define SPEECH_HELLO 0x351A

extern const unsigned char sample[];
extern const int sample_len;


int main(char* args) {
  if (!detect_speech()) {
    fc_tputs("No speech synthesizer detected :(\n");
    return 0;
  }

  say_vocab(SPEECH_HELLO);
  speech_wait();

  speech_reset();
  say_data(sample, sample_len);
  speech_wait();

  return 0;
}
