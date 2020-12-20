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
  } else if (matchcmd(tok, "history")) {
    wraptext("==History==\n\n"
      "history\n\n"
      "List command history. You may then recall an item by number with !n, such as !7 to copy the 7th "
      "previous command into the current command line.\n"
      "Also, Fctn-8(redo) and Fctn-E(up arrow) moves up through history. Fctn-X(down arrow) moves back down.\n"
      "32K systems only have 1 history entry. SAMS systems have 4K of history.\n"
    );
  } else if (matchcmd(tok, "bar")) {
    wraptext("==Display Bar==\n\n"
      "bar <on|off>\n\n"
      "Turn display bar on or off. Bar contains title and clock on top line of screen.\n"
    );
  } else if (matchcmd(tok, "date")) {
    wraptext("==Date==\n\n"
      "date\n\n"
      "Print the date and time.\n"
      "Clock source is based on value of 'CLOCK' environment variable. Supported values are:\n"
      "- PI.CLOCK (default)\n"
      "- CLOCK\n"
      "- IDE.TIME\n"
    );
  } else if (matchcmd(tok, "sysinfo")) {
    wraptext("==SysInfo==\n\n"
      "sysinfo\n\n"
      "Print details about the current system hardware configuration.\n"
    );
  } else if (matchcmd(tok, "pwd")) {
    wraptext("==PWD==\n\n"
      "pwd\n\n"
      "Print current working directory.\n"
    );
  } else {
    wraptext("no help for command: ");
    bk_tputs_ram(tok);
    bk_tputc('\n');
  }
  bk_tputc('\n');
}
