#include "banks.h"
#define MYBANK BANK(2)

#include "debug_vdp.h"
#include "detect_vdp.h"
#include "terminal.h"
#include "strutil.h"
#include <vdp.h>

/*
 * Select a status register with R#15 and read it. R#15 is restored to 0 so
 * the console's normal status reads are not disturbed.
 */
static unsigned int read_status_register(int reg) {
  unsigned int value;
  VDP_SET_REGISTER(0x0f, reg & 0x0f);
  value = VDPST;
  VDP_SET_REGISTER(0x0f, 0);
  return value;
}

/*
 * F18A register readback: select SR15, set a read address of >RR00, then read
 * the status port. This is only meaningful on an F18A that is already
 * unlocked; it does not unlock the VDP.
 */
static unsigned int read_control_register(int reg) {
  unsigned int value;
  VDP_SET_REGISTER(0x0f, 0x0f);
  VDP_SET_ADDRESS((unsigned int)(reg & 0x3f) << 8);
  value = VDPST;
  VDP_SET_REGISTER(0x0f, 0);
  return value;
}

static void print_byte(unsigned int value) {
  bk_tputs_ram(bk_uint2hex(value & 0xff) + 2);
}

static void print_register(char prefix, int reg, unsigned int value) {
  bk_tputc(prefix);
  print_byte(reg);
  tputs_rom(": ");
  print_byte(value);
}

static void dump_registers(char prefix, int count, int status) {
  int reg;
  for (reg = 0; reg < count; reg++) {
    if ((reg & 7) == 0) {
      if (reg != 0) {
        bk_tputc('\n');
      }
      bk_tputc(prefix);
      print_byte(reg);
      tputs_rom(": ");
    }
    print_byte(status ? read_status_register(reg) : read_control_register(reg));
    bk_tputc(' ');
  }
  bk_tputc('\n');
}

static void show_register(char prefix, int reg, int status) {
  print_register(prefix, reg,
                 status ? read_status_register(reg) : read_control_register(reg));
  bk_tputc('\n');
}

void debug_vdp_command(char* tok) {
  char cmd = tok[0];
  char* arg = tok + 1;
  int reg = -1;

  if (*arg == 0) {
    arg = bk_strtok(0, ' ');
  }
  if (arg && *arg) {
    reg = bk_htoi(arg);
  }

  if (cmd == 'S') {
    if (reg < 0) {
      dump_registers('S', 16, 1);
    } else {
      show_register('S', reg & 0x0f, 1);
    }
    return;
  }

  // 'V': control registers.
  if (reg < 0) {
    tputs_rom("VDP type: ");
    bk_tputs_ram(bk_uint2hex(vdp_type));
    tputs_rom("\nregisters (SR15, no unlock):\n");
    dump_registers('V', 64, 0);
  } else {
    show_register('V', reg & 0x3f, 0);
  }
}
