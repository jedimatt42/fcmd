#include "banks.h"
#define MYBANK BANK(6)

#include "commands.h"
#include "main.h"
#include "globals.h"
#include "strutil.h"
#include "terminal.h"
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

int moreHelp2(char* tok) {
  if (matchcmd(tok, "f18a")) {
    wraptext("==F18A==\n\n"
      "f18a\n\n"
      "Switch to the F18A personality and initialize its 80x30 color text mode.\n\n"
      "ForceCommand does not probe for an F18A when a V9938 or V9958 is detected, "
      "so this command can be used to select F18A mode from an AUTOCMD script.\n\n"
      "On a V9938/F18A hybrid this permanently switches the card to F18A mode. "
      "A power cycle is required to return to the V9938 personality.\n\n"
      "Prints an error if no F18A is detected.\n"
    );
    return 1;
  }

  if (matchcmd(tok, "rdpart")) {
    wraptext("==Myarc RD Partition==\n\n"
      "rdpart <ram> <disk> <spool>\n\n"
      "Configure Myarc EXP-1 Ramdisk memory allocations.\n"
      "<ram> - 32 for use as 32K memory expansion, or 128 for 128KOS support.\n"
      "<drive> - size of ramdisk allocation in kilobytes.\n"
      "<spool> - size of print spool allocation in kilobytes.\n"
    );
    return 1;
  }

  if (matchcmd(tok, "rdemdk")) {
    wraptext("==Myarc RD Emulate DSK==\n\n"
      "rdemdk <drive>\n\n"
      "Configure Myarc EXP-1 Ramdisk drive number.\n"
      "<drive> - number 1 - 5 to emulate DSK1 through DSK5\n"
    );
    return 1;
  }

  if (matchcmd(tok, "rdvol")) {
    wraptext("==Myarc RD Volume Name==\n\n"
      "rdvol <name>\n\n"
      "Configure Myarc EXP-1 Ramdisk volume name.\n"
      "<name> - upto 10 character volume label.\n"
    );
    return 1;
  }

  if (matchcmd(tok, "redirect")) {
    wraptext("==Output Redirection==\n\n"
      "command >filepath\n"
      "command >>filepath\n\n"
      "Write terminal output to a DISPLAY/VARIABLE 80 file.\n"
      "Records break at 80 characters, line feed, carriage return, or CRLF.\n"
      "> replaces an existing file. >> appends; an existing file must already be DISPLAY/VARIABLE 80.\n"
      "Use \\> for a literal > character.\n"
      "Only output written through the ForceCommand terminal layer is captured.\n"
      "The STATUS variable is set to the command exit status.\n"
      "See 'help pipeline' for | pipelines.\n"
    );
    return 1;
  }

  return 0;
}
