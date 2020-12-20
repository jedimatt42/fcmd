#include "banks.h"
#define MYBANK BANK(5)

#include <string.h>
#include "b0_globals.h"
#include "b5_prompt.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b5_clock.h"

void print_prompt(char* pattern) {
  int i = 0;
  int plen = bk_strlen(pattern);
  while(i < plen) {
    if (pattern[i] == '\\') {
      i++;
      char val = pattern[i];
      switch(val) {
        case 0:
          return;
        case 'c':
          bk_tputs_ram(bk_uint2hex(currentDsr->crubase));
          bk_tputc('.');
          break;
        case 'p':
          bk_tputs_ram(currentPath);
          break;
        case 'g':
          bk_tputc('>');
          break;
        case 'b':
          bk_tputc(0x08);
          break;
        case 'n':
          bk_tputc(0X0a);
          break;
        case 'r':
          bk_tputc('|');
          break;
        case 's':
          bk_tputc(' ');
          break;
        case 't':
          {
            struct DateTime dt;
            datetime(&dt);
            pretty_time(&dt);
          }
          break;
        default:
          bk_tputc(val);
          break;
      }
    } else {
      bk_tputc(pattern[i]);
    }
    i++;
  }
}
