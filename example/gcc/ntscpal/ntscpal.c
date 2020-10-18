#include <fc_api.h>

// I see 5450 for PAL, and 4541 for NTSC, split the difference
#define PAL_THRESHOLD 5000

char *uint2str(unsigned int x) {
  static char strbuf[8];

  // we just populate and return strbuf
  int unzero = 0;
  char *out = strbuf;
  int div = 10000;
  int tmp;

  while (div > 0) {
    tmp = x / div;
    x = x % div;
    if ((tmp > 0) || (unzero)) {
      unzero = 1;
      *(out++) = tmp + '0';
    }
    div /= 10;
  }
  if (!unzero) *(out++) = '0';
  *out = '\0';
  return strbuf;
}

int isPal() {
  register int cycles = 0;

  // wait for VDP interrupt
  __asm__(
      "limi 0\n\t"
      "clr r12\n\t"
      "tb 2\n\t"
      "jeq -4\n\t"
      "movb @>8802,r12"
      :
      :
      : "r12");
  // again, provides a normalized result
  __asm__(
      "limi 0\n\t"
      "clr r12\n\t"
      "tb 2\n\t"
      "jeq -4\n\t"
      "movb @>8802,r12"
      :
      :
      : "r12");

  // now wait again, but count while we wait.
  // and repeat this 4 times, a fraction of a second
  // as it is not precise
  for(int i=0; i<4; i++) {
    __asm__(
        "clr r12\n\t"
        "inc %0\n\t"
        "tb 2\n\t"
        "jeq -6\n\t"
        "movb @>8802,r12"
        : "=r"(cycles)
        : "r" (cycles)
        : "r12");
  }

  fc_tputs(uint2str(cycles));

  return cycles >= PAL_THRESHOLD;
}

int main(char* args) {
  fc_tputs("detecting system type... ");

  if (isPal()) {
    fc_tputs(" PAL");
  } else {
    fc_tputs(" NTSC");
  }
  fc_tputc('\n');

  return 0;
}
