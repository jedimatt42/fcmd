#include "banks.h"
#define MYBANK BANK(11)

#include "moreHelpHeight.h"
#include "strutil.h"
#include "terminal.h"

int moreHelpHeight(char* tok) {
  if (0 != bk_strcmpi(str2ram("height"), tok)) {
    return 0;
  }

  tputs_rom("==Set Screen Height==\n\n"
            "height <24|26|30>\n\n"
            "24 all modes, 26 9938/9958, 30 F18A.\n");
  bk_tputc('\n');
  return 1;
}
