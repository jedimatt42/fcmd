#include "banks.h"
#define MYBANK BANK(15)

#include "commands.h"
#include "b0_main.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b14_moreHelp.h"
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
void handleHelp() {
  char* tok = bk_strtok(0, ' ');
  if (tok == 0) {
    wraptext("==Help==\n\n"
    "help <command>\n\n"
    "Commands:\n\n"
    "alias autocmd bar cd cfmount checksum cls color copy date "
    "delete drives echo ed env exit fg99 format goto help "
    "history if "
    "label load lvl2 mkdir palette path protect pwd readkey rename "
    "rdemdk rdpart rdvol "
    "rmdir sysinfo tipibeeps tipihalt tipimap "
    "tipireboot type unprotect ver width xb\n");
    return;
  }

  if (matchcmd(tok,"autocmd")) {
    wraptext("==AUTOCMD==\n\n"
    "Special file: AUTOCMD - First drive will be scanned for existance of script AUTOCMD. If present it will run.\n\n"
    "Changes when AUTOCMD is found:\n"
    "- startup banner is suppressed.\n"
    "- initial tipibeeps sound is suppressed.\n");
  } else if (matchcmd(tok, "cd")) {
    wraptext("==Change Directory==\n\n"
    "cd <path>|..\n\n"
    "path - A device or directory path. The path may be a subdirectory of the current location, or a fully qualified path starting with a device name or crubase and device name\n"
    "..   - go to parent directory\n");
  } else if (matchcmd(tok, "cfmount")) {
    wraptext("==Mount Compact Flash Volumes==\n\n"
    "cfmount\n"
    "cfmount /v [begin end]\n"
    "cfmount [/p] <drive> <volume>\n\n"
    "With no arguments, list current mapping info\n"
    "/v - optional, list all volumes with an option range of volumes specified.\n"
    "/p - optional, persist drive mapping\n"
    "drive - (1,2,3) DSK device unit number for volume mapping\n"
    "volume - compact flash volume number to map\n");
  } else if (matchcmd(tok, "checksum")) {
    wraptext("==File Checksum==\n\n"
    "checksum <filepath>\n\n"
    "Calculate 16 bit checksum of a file\n");
  } else if (matchcmd(tok, "cls")) {
    wraptext("==Clear Screen==\n\n"
    "cls\n\n"
    "Clear the screen and relocate cursor to upper left\n");
  } else if (matchcmd(tok, "color")) {
    wraptext("==Set Color==\n\n"
    "color <F> [B]\n\n"
    "Set text and background color (0-15)\n\n"
    "F - foreground text color\n"
    "B - optional, background color\n\n"
    "In F18A 80 column mode, this sets colors for subsequent text, and current border.\n\n"
    "In in all other modes, it sets the full screen colors.\n\n"
    "colors follow TI BASIC numbering\n");
  } else if (matchcmd(tok, "copy")) {
    wraptext("==Copy Files==\n\n"
    "copy <fileset> <path>\n\n"
    "Copy a file set to a directory or device\n"
    "fileset - path and file pattern for source files to copy. Wildcard * may be at beginning or end of name but not both\n\n"
    "examples: DSK1.*BAS, DSK1.MUNCHM*, TELN*, *S\n\n"
    "path - destination device and directory. If path includes a new filename then wildcard is not supported in fileset.\n");
  } else if (matchcmd(tok, "delete")) {
    wraptext("==Delete File==\n\n"
    "delete [/y] <fileset>\n\n"
    "Delete a file set\n\n"
    "/y      - optional, do not prompt for each file.\n"
    "fileset - path and file pattern for source files to copy. Wildcard * may be at beginning or end of name but not both\n"
    "examples: DSK1.*BAS, DSK1.MUNCHM*, TELN*, *S\n");
  } else if (matchcmd(tok, "dir")) {
    wraptext("==List Directory==\n\n"
    "dir [/w] [path]\n\n"
    "List current directory or directory for path\n\n"
    "/w - optional. Output a simplified listing in multiple columns\n\n"
    "path - full or relative path to directory or file pattern. Wildcard * may be at beginning or end of name but not both\n\n"
    "examples: DSK1.*BAS, DSK1.MUNCHM*, TELN*, *S\n");
  } else if (matchcmd(tok, "drives")) {
    wraptext("==List Drives==\n\n"
    "drives\n\n"
    "List device names grouped by CRU base address\n");
  } else if (matchcmd(tok, "echo")) {
    wraptext("==Echo to Terminal==\n\n"
    "echo [/n] [text]\n\n"
    "Print text to ANSI terminal screen\n\n"
    "/n : optional, do not print newline after text\n\n"
    "substitutions: ");
    bk_tputc(92); // a FS - File Separator '\' character.
    wraptext("e will output an ESCAPE, ascii 27 character\n");
  } else if (matchcmd(tok, "env")) {
    wraptext("==Environment Variables==\n\n"
    "env\n\n"
    "List all variables and their values\n\n"
    "Variables are set by assigning a name of up to 10 characters a value, or by specific commands.\n\n"
    "example: NAME1=FLINSTONE\n"
    "Variable names are case insensitive.\n"
    "Variables may be used in any command by the '$' + <name> format.\n\n"
    "You may disambiguate with the '$' '(' <name ')' syntax.\n\n"
    "examples:\n"
    "echo $NAME1\n"
    "echo $(NAME1)A99\n");
  } else if (matchcmd(tok, "exit")) {
    wraptext("==Exit Force Command==\n\n"
    "exit\n\n"
    "Quit Force Command. If you are running in the auto start mode, Force Command will restart\n");
  } else if (matchcmd(tok, "fg99")) {
    wraptext("==FinalGROM==\n\n"
    "fg99 <cart>\n\n"
    "Load cartridge from FinalGROM99 sd card\n\n"
    "cart - maximum 8 character name without the '.bin'\n\n"
    "Cart must be in the same directory on the sd card as Force Command\n");
  } else if (matchcmd(tok, "goto")) {
    wraptext("==Goto==\n\n"
    "goto <label>\n\n"
    "Jump to the line following a label\n");
  } else if (matchcmd(tok, "help")) {
    wraptext("==Help==\n\n"
    "help <command>\n\n"
    "Show help for individual command. Command names and options are case "
    "insensitive. Device, file and directory name or path parameters are case sensitive.\n");
  } else if (matchcmd(tok, "if")) {
    wraptext("==Conditional==\n\n"
    "if <expr> then <command>\n\n"
    "Expression following 'if' is evaluated to true or false. If the expression "
    "evaluates to true, the rest of the command is executed.\n"
    "example:\n\n"
    "if $A == C then goto LC\n\n"
    "expressions syntax:\n"
    "[not] <lvalue> ' ' <operator> ' ' <rvalue>\n\n"
    "lvalue - quoted string which may contain variable references\n"
    "rvalue - quoted string which may contain variable references\n"
    "operator - '=='\n");
  } else if (matchcmd(tok,"label")) {
    wraptext("==Label==\n\n"
    "<label-name>:\n\n"
    "A custom identifier followed by a colon defines a label that may be branched to using 'goto'\n");
  } else if (matchcmd(tok,"load")) {
    wraptext("==Load EA5 Program==\n\n"
    "load <filepath>\n\n"
    "Load and run an EA5 program image file or files\n");
  } else if (matchcmd(tok,"lvl2")) {
    wraptext("==List IO Subroutines==\n\n"
    "lvl2 <crubase>\n\n"
    "List level 2 I/O subprograms in DSR ROM at the specified CRU base address\n");
  } else if (matchcmd(tok,"mkdir")) {
    wraptext("==Make Directory==\n\n"
    "mkdir <pathname>\n\n"
    "Create new directory with the given path name\n");
  } else if (matchcmd(tok,"path")) {
    wraptext("==PATH environment variable==\n\n"
    "PATH variable\n\n"
    "';' seperated list of device or directory names to search in for scripts and executable commands\n\n"
    "PATH=TIPI.FC.;DSK5.\n");
  } else if (matchcmd(tok,"protect")) {
    wraptext("==Protect File==\n\n"
    "protect <filepath>\n\n"
    "Set protect flag for the specified file path\n");
  } else if (matchcmd(tok,"readkey")) {
    wraptext("==Read Keyboard==\n\n"
    "readkey <varname>\n\n"
    "Set varname to value of key currently pressed\n");
  } else if (matchcmd(tok,"rename")) {
    wraptext("==Rename File==\n\n"
    "rename <old-name> <new-name>\n\n"
    "Rename a file or directory at the current location\n");
  } else if (matchcmd(tok,"rmdir")) {
    wraptext("==Remove Directory==\n\n"
    "rmdir <dirpath>\n\n"
    "Remove a given directory path\n");
  } else if (matchcmd(tok,"tipibeeps")) {
    wraptext("==Play TIPI Beeps==\n\n"
    "tipibeeps\n\n"
    "Play dih-dah-dih sound list that used to be played by TIPI powerup routine\n");
  } else if (matchcmd(tok,"tipihalt")) {
    wraptext("==Halt TIPI Raspberry PI==\n\n"
    "tipihalt\n\n"
    "Shutdown Raspberry PI operating system\n");
  } else if (matchcmd(tok,"tipimap")) {
    wraptext("==TIPI DSK Mapping==\n\n"
    "tipimap auto [on|off]\n"
    "tipimap [/c] <drive> [path]\n\n"
    "Set or get a tipi drive mapping. If no drive is specified, all configuration items are listed\n"
    "/c - optional. unmap drive\n"
    "drive - the name of the drive\n"
    "path - the directory to set\n"
    "auto - set to on or off to map DSK1 when PROGRAM image is loaded\n");
  } else if (matchcmd(tok,"tipireboot")) {
    wraptext("==Reboot TIPI Raspberry PI==\n\n"
    "tipireboot\n\n"
    "Reboot Raspberry PI operating system\n");
  } else if (matchcmd(tok,"type")) {
    wraptext("==Type File to Terminal==\n\n"
    "type [/ansi] <filepath>\n\n"
    "Copy the contents of the file to the screen\n\n"
    "/ansi - optional. Open in DISPLAY/FIXED mode instead of DISPLAY/VARIABLE and terminate output on CTRL-Z character (26)\n");
  } else if (matchcmd(tok,"unprotect")) {
    wraptext("==Unprotect File==\n\n"
    "unprotect <filepath>\n\n"
    "Remove protect flag for the specified file path\n");
  } else if (matchcmd(tok,"ver")) {
    wraptext("==Print Version==\n\n"
    "ver\n\n"
    "Display Force Command version information\n");
  } else if (matchcmd(tok,"width")) {
    wraptext("==Set Screen Width==\n\n"
    "width <40|80>\n\n"
    "Change display to 40x24, 80x26 or 80x30 mode.\n"
    "F18A VDP will provide 80x30 with colors per character.\n"
    "9938/9958 VDPs will provide 80x26 with single foreground/background colors.\n"
    "Other VDPs will fall back on 40x24 with single foreground/background colors.\n");
  } else {
    bk_b14Help(tok);
  }
  bk_tputc('\n');
}
