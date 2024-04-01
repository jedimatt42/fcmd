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
  } else if (matchcmd(tok, "ed")) {
    wraptext("==Edit DV80==\n\n"
      "ed <filepath>\n\n"
      "Open or create a file in the builtin file editor. Press F7(Aid) in editor for editor details\n"
    );
  } else if (matchcmd(tok, "palette")) {
    wraptext("==Palette==\n\n"
      "palette /r\n"
      "palette <rgb-0> <rgb-1> ... <rgb-15>\n\n"
      "Reset or set the first 16 F18A palette registers.\n"
      "/r - reset to standard TMS9918A palette\n"
      "<rgb-0..15> - 16 12bit hex value that define the palette colors.\n"
    );
  } else if (matchcmd(tok, "rdpart")) {
    wraptext("==Myarc RD Partition==\n\n"
      "rdpart <ram> <disk> <spool>\n\n"
      "Configure Myarc EXP-1 Ramdisk memory allocations.\n"
      "<ram> - 32 for use as 32K memory expansion, or 128 for 128KOS support.\n"
      "<drive> - size of ramdisk allocation in kilobytes.\n"
      "<spool> - size of print spool allocation in kilobytes.\n"
    );
  } else if (matchcmd(tok, "rdemdk")) {
    wraptext("==Myarc RD Emulate DSK==\n\n"
      "rdemdk <drive>\n\n"
      "Configure Myarc EXP-1 Ramdisk drive number.\n"
      "<drive> - number 1 - 5 to emulate DSK1 through DSK5\n"
    );
  } else if (matchcmd(tok, "rdvol")) {
    wraptext("==Myarc RD Volume Name==\n\n"
      "rdvol <name>\n\n"
      "Configure Myarc EXP-1 Ramdisk volume name.\n"
      "<name> - upto 10 character volume label.\n"
    );
  } else if (matchcmd(tok, "format")) {
    wraptext("==Floppy Format==\n\n"
      "format [/ss] [/ds] [/sd] [/dd] [/40] [/80] [/i <interleave>] [/v <volumename>] <drive>\n\n"
      "Format a floppy disk\n"
      "[/ss] - single sided (default)\n"
      "[/ds] - double sided\n"
      "[/sd] - single density (default)\n"
      "[/dd] - double density\n"
      "[/40] - 40 track (default)\n"
      "[/80] - 80 track\n"
      "[/i <interleave>] - for supportted controllers, set non-default interleave\n"
      "[/v <volumename>] - disk volume name, 1-10 characters (default is empty)\n"
      "<drive> - the disk device name to format. DSK1., 1100.DSK1.\n"
      "The crubase is forced to 1100. as only floppy controllers are supported.\n"
    );
  } else {
    wraptext("no help for command: ");
    bk_tputs_ram(tok);
    bk_tputc('\n');
  }
  bk_tputc('\n');
}
