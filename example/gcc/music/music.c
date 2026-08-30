#include "fc_api.h"

#define SOUND		*((volatile unsigned char*)0x8400)

#define CH_1 0x8000
#define CH_2 0xA000
#define CH_3 0xC000


#define A_1 0x093F
#define B_1 0x0A38

void play(int c, int n, char v) {
  register int t = c | n;
  SOUND = t >> 8;
  SOUND = t & 0x00ff;
  SOUND = v;
}

void delay(int duration) {
  volatile int t = duration;
  while(t > 0) {
    t--;
  }
}

void sound_off() {
  SOUND = 0x9F;
  SOUND = 0xBF;
  SOUND = 0xDF;
  SOUND = 0xFF;
}

int fc_main(char* args) {
  play(CH_1, A_1, 0x92);
  delay(10000);
  play(CH_1, A_1, 0x92);
  play(CH_2, B_1, 0x94);
  delay(10000);
  play(CH_1, B_1, 0x94);
  delay(10000);
  sound_off();

  return 0;
}
