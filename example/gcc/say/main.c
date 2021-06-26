#include "fc_api.h"

// ------------ borrowed from libti99 --------------
// Read Data
#define VDPRD   *((volatile unsigned char*)0x8800)
// Read Status
#define VDPST   *((volatile unsigned char*)0x8802)
// Write Address/Register
#define VDPWA   *((volatile unsigned char*)0x8C02)
// Write Data
#define VDPWD   *((volatile unsigned char*)0x8C00)

// Set VDP address for read (no bit added)
inline void VDP_SET_ADDRESS(unsigned int x) { VDPWA = ((x) & 0xff); VDPWA = ((x) >> 8); }
// -------------------------------------------------

  // use upper expansion as our buffer (after we take 8k for our own headroom)
unsigned char* sample = (char*)0xC000;
int sample_len = 0;

unsigned char get_nibble(unsigned char c, int* state) {
  (*state)++;
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else if (c >= '0' && c <= '9') {
    return c - '0';
  }
  *state = 0;
  return 0;
}

void __attribute__((noinline)) read_sample_line(int addr, int len) {
  // addr is VDP RAM address containing record from file, of len characters.
  VDP_SET_ADDRESS(addr);
  int state = 0;
  while(len) {
    len--;
    unsigned char nibble = get_nibble(VDPRD, &state);
    switch(state) {
      case 1:
        sample[sample_len] = nibble << 4;
        break;
      case 2:
        sample[sample_len++] |= nibble;
        break;
      default:
        break;
    }
  }
}

void __attribute__((noinline)) load_sample(char* fname_buffer, struct DeviceServiceRoutine* dsr) {
  struct PAB pab;

  int ferr = fc_dsr_open(dsr, &pab, fname_buffer, DSR_TYPE_INPUT | DSR_TYPE_VARIABLE, 0);
  if (ferr) {
    fc_tputs("error opening file\n");
    return;
  }

  while (!ferr) {
    ferr = fc_dsr_read(dsr, &pab, 0);
    if (!ferr) {
      read_sample_line(pab.VDPBuffer, pab.CharCount);
    }
  }

  fc_dsr_close(dsr, &pab);
}

void __attribute__((noinline)) play_sample() {
  if (sample_len > 0) {
    fc_speech_reset();
    fc_say_data(sample, sample_len);
    fc_speech_wait();
  }
}

int main(char* args) {
  // expect DV80 file of hex data
  struct DeviceServiceRoutine* dsr;
  char fname_buffer[30];
  fc_parse_path_param(args, &dsr, fname_buffer, PR_REQUIRED);
  if (dsr == 0) {
    fc_tputs("error no speech file specified\n\n");
    fc_tputs("SAY <file>\n");
  } else {
    load_sample(fname_buffer, dsr);
    if (!sample_len) {
      fc_tputs("Error: No speech data loaded\n");
    } else
    if (!fc_detect_speech()) {
      fc_tputs("Error: No speech synthesizer detected\n");
    } else {
      play_sample();
    }
  }
  return 0;
}
