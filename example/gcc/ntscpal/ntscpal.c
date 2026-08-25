#include <fc_api.h>

int main(char* args) {
  struct GfxInformation info;

  gfx_get_info(&info);

  term_puts("Display: ");
  if (info.width == 40) {
    term_puts("40x");
  }
  if (info.width == 80) {
    term_puts("80x");
  }
  if (info.height == 30) {
    term_puts("30, ");
  }
  if (info.height == 24) {
    term_puts("24, ");
  }

  if (info.isPal) {
    term_puts("PAL\n");
  } else {
    term_puts("NTSC\n");
  }



  return 0;
}
