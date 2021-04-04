#include "fc_api.h"
#include "speech.h"

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

void __attribute__((noinline)) vdp_to_ram(int vdp_addr, char* cpu_addr, int len) {
  VDP_SET_ADDRESS(vdp_addr);
  while(len > 0) {
    *cpu_addr++ = VDPRD;
    len--;
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
      int len = pab.CharCount;
      char line[len + 1];
      // read a line
      vdp_to_ram(pab.VDPBuffer, line, len);
      line[len] = 0;
    }
  }

  fc_dsr_close(dsr, &pab);
}

void __attribute__((noinline)) play_sample() {
  if (sample_len > 0) {
    speech_reset();
    say_data(sample, sample_len);
    speech_wait();
  }
}

int main(char* args) {
  if (!detect_speech()) {
    fc_tputs("No speech synthesizer detected :(\n");
    return 0;
  }

  // expect DV80 file of hex data
  struct DeviceServiceRoutine* dsr;
  char fname_buffer[30];
  fc_parse_path_param(args, &dsr, fname_buffer, PR_REQUIRED);
  if (dsr == 0) {
    fc_tputs("error no speech file specified\n\n");
    fc_tputs("SAY <file>\n");
  } else {
    load_sample(fname_buffer, dsr);
    play_sample();
  }

  return 0;
}
