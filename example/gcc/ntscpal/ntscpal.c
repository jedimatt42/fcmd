#include <fc_api.h>

int main(char* args) {
  struct DisplayInformation info;

  sys_display_info(&info);

  term_puts("Display: ");
  if (info.displayWidth == 40) {
    term_puts("40x");
  }
  if (info.displayWidth == 80) {
    term_puts("80x");
  }
  if (info.displayHeight == 30) {
    term_puts("30, ");
  }
  if (info.displayHeight == 24) {
    term_puts("24, ");
  }

  if (info.isPal) {
    term_puts("PAL\n");
  } else {
    term_puts("NTSC\n");
  }



  return 0;
}
