#include <fc_api.h>

int main(char* args) {
  struct SystemInformation info;

  fc_sys_info(&info);

  fc_tputs("Display: ");
  if (info.displayWidth == 40) {
    fc_tputs("40x");
  }
  if (info.displayWidth == 80) {
    fc_tputs("80x");
  }
  if (info.displayHeight == 30) {
    fc_tputs("30, ");
  }
  if (info.displayHeight == 24) {
    fc_tputs("24, ");
  }

  if (info.isPal) {
    fc_tputs("PAL\n");
  } else {
    fc_tputs("NTSC\n");
  }



  return 0;
}
