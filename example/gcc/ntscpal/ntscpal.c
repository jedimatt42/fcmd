#include <fc_api.h>

int fc_main(char* args) {
  struct DisplayInformation info;

  fc_sys_display_info(&info);

  fc_term_puts("Display: ");
  if (info.displayWidth == 40) {
    fc_term_puts("40x");
  }
  if (info.displayWidth == 80) {
    fc_term_puts("80x");
  }
  if (info.displayHeight == 30) {
    fc_term_puts("30, ");
  }
  if (info.displayHeight == 24) {
    fc_term_puts("24, ");
  }

  if (info.isPal) {
    fc_term_puts("PAL\n");
  } else {
    fc_term_puts("NTSC\n");
  }



  return 0;
}
