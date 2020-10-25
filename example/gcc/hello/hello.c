#include "fc_api.h"

void cmdline_args(char *args) {
  fc_tputs("Args: ");
  fc_tputs(args);
  fc_tputc('\n');
}

void sams_example() {
  int page_start = fc_sams_alloc_pages(1);
  if (page_start != 8) {
    fc_tputs("page not allocated\n");
  } else {
    fc_tputs("page allocated\n");
  }
  *(volatile int *)(0xE000) = 0xABCD;
  fc_sams_map_page(page_start, 0xE000);
  if (*(volatile int *)(0xE000) == 0xABCD) {
    fc_tputs("page not mapped\n");
  } else {
    fc_tputs("page mapped\n");
  }
  fc_sams_map_page(6, 0xE000);
  fc_sams_free_pages(1);
  if (*(volatile int *)(0xE000) != 0xABCD) {
    fc_tputs("page not unmapped\n");
  } else {
    fc_tputs("page unmapped\n");
  }
}

void input_output() {
  fc_tputs("\nHello, what is your name? ");
  char name[14];
  for (int i = 0; i < 14; i++) {
    name[i] = 0;
  }
  fc_getstr(name, 13, 1);
  fc_tputs("\nHello ");
  fc_tputs(name);
  fc_tputs("!\nNice to meet you.\n");
}

int hexdigit(int v) {
   if (v > 9) {
      return v + 'A' - 10;
   } else {
      return v + '0';
   }
}

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

void info() {
  struct DisplayInformation dInfo;
  struct SamsInformation sInfo;

  fc_sams_info(&sInfo);
  fc_display_info(&dInfo);

  fc_tputs("VDP: ");
  switch(dInfo.vdp_type) {
    case VDP_9918:
      fc_tputs("TMS 9918\n");
      break;
    case VDP_9938:
      fc_tputs("Yamaha 9938\n");
      break;
    case VDP_9958:
      fc_tputs("Yamaha 9958\n");
      break;
    case VDP_F18A:
      fc_tputs("F18A\n");
      break;
  }

  fc_tputs("Display: ");
  if (dInfo.isPal) {
    fc_tputs("PAL\n");
  } else {
    fc_tputs("NTSC\n");
  }

  fc_tputs("Memory: ");
  if (sInfo.total_pages == 0) {
    fc_tputs("32k\n");
  } else {
    fc_tputs(uint2str(4*sInfo.total_pages));
    fc_tputs("K\n pages used: ");
    fc_tputs(uint2str(sInfo.next_page));
    fc_tputc('\n');
  }
}

int main(char* args) {
  info();

  char buf[2];
  buf[0] = 0;
  fc_getstr(buf, 1, 0);

  if (args && args[0]) {
    return 0;
  }

  fc_exec("HELLO stop");

  return 0;
}
