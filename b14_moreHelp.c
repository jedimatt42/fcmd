#include "banks.h"
#define MYBANK BANK(14)

#include "commands.h"
#include "b0_main.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include <string.h>
#include <conio.h>

static int matchcmd(char* input, char* exp) {
  char stackstr[80];
  int len = 0;
  while(exp[len] != 0) {
    stackstr[len] = exp[len];
    len++;
  }
  stackstr[len] = 0;
  return 0 == bk_strcmpi(input, stackstr);
}

static int wordlen(char* str) {
  int r=0;
  while(!(str[r] == 0 || str[r] == ' ')) {
    r++;
  }
  return r;
}

static void wraptext(char* str) {
  int i=0;
  while(str[i] != 0) {
    if (str[i] == ' ') {
      if ((1 + wherex() + wordlen(str + (i+1))) >= displayWidth) {
        bk_tputc('\n');
      } else {
        bk_tputc(str[i]);
      }
    } else {
      bk_tputc(str[i]);
    }
    i++;
  }
}

/* ##########|----.----1----.----2----.----3----.----4|######## */
void b14Help(char* tok) {
if (matchcmd(tok, "xb")) {
  wraptext("==Run XB program==\n\n"
    "xb <program-path>\n\n"
    "Switch a FinalGROM99 to an Extended BASIC cartridge, configured to RUN the specified program.\n"
    "Default cartridge name is TIXB_G with start address 25474.\n"
    "Use variable XBMOD to override cartridge name, and XBADDR to override start address.\n");
  } else {
    wraptext("no help for command: ");
    bk_tputs_ram(tok);
    bk_tputc('\n');
  }
  bk_tputc('\n');
}
