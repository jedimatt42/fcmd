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

int main(char* args) {
  cmdline_args(args);
  //sams_example();
  //input_output();

  fc_exec("echo hello world");
  fc_exec("HELLO");

  return 0;
}
