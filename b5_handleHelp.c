#include "banks.h"
#define MYBANK BANK_5

#include "commands.h"
#include "b0_main.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include <string.h>
#include <conio.h>

int matchcmd(char* input, char* exp) {
  char stackstr[80];
  int len = 0;
  while(exp[len] != 0) {
    stackstr[len] = exp[len];
    len++;
  }
  stackstr[len] = 0;
  return 0 == strcmpi(input, stackstr);
}

int wordlen(char* str) {
  int r=0;
  while(!(str[r] == 0 || str[r] == ' ')) {
    r++;
  }
  return r;
}

void wraptext(char* str) {
  int i=0;
  while(str[i] != 0) {
    if (str[i] == ' ') {
      if ((1 + wherex() + wordlen(str + (i+1))) >= displayWidth) {
        tputc('\n');
      } else {
        tputc(str[i]);
      }
    } else {
      tputc(str[i]);
    }
    i++;
  }
}

/* ##########|----.----1----.----2----.----3----.----4|######## */
void handleHelp() {
  char* tok = strtok(0, " ");
  if (tok == 0) {
    wraptext("==Help==\n\n");
    wraptext("help <command>\n\n");
    wraptext("Commands:\n\n");
    wraptext("call cd checksum cls color copy delete drives echo env exit fg99 ftp goto help if label load lvl2 mkdir protect readkey rename rmdir tipibeeps tipihalt tipimap tipireboot type unprotect ver width\n");
    return;
  }

  if (matchcmd(tok,"call")) {
    wraptext("==Run a script==\n\n");
    wraptext("call <filepath>\n\n");
    wraptext("File must be DISPLAY VARIABLE file.\n\n");
    wraptext("Special file: AUTOCMD - First drive will be scanned for existance of AUTOCMD. If present it will run.\n\n");
    wraptext("Changes when AUTOCMD is found:\n");
    wraptext("- startup banner is suppressed.\n");
    wraptext("- initial tipibeeps sound is suppressed.\n");
  } else if (matchcmd(tok, "cd")) {
    wraptext("==Change Directory==\n\n");
    wraptext("cd <path>|..\n\n");
    wraptext("path - A device or directory path. The path may be a subdirectory of the current location, or a fully qualified path starting with a device name or crubase and device name\n");
    wraptext("..   - go to parent directory\n");
  } else if (matchcmd(tok, "checksum")) {
    wraptext("==File Checksum==\n\n");
    wraptext("checksum <file>\n\n");
    wraptext("Calculate 16 bit checksum of a file in the current directory\n");
  } else if (matchcmd(tok, "cls")) {
    wraptext("==Clear Screen==\n\n");
    wraptext("cls\n\n");
    wraptext("Clear the screen and relocate cursor to upper left\n");
  } else if (matchcmd(tok, "color")) {
    wraptext("==Set Color==\n\n");
    wraptext("color <F> [B]\n\n");
    wraptext("Set text and background color (0-15)\n\n");
    wraptext("F - foreground text color\n");
    wraptext("B - optional, background color\n\n");
    wraptext("In 80 column mode, this sets colors for subsequent text, and current border.\n\n");
    wraptext("In 40 column mode, it sets the full screen colors.\n\n");
    wraptext("colors follow TI BASIC numbering\n");
  } else if (matchcmd(tok, "copy")) {
    wraptext("==Copy Files==\n\n");
    wraptext("copy <fileset> <path>\n\n");
    wraptext("Copy a file set to a directory or device\n");
    wraptext("fileset - path and file pattern for source files to copy. Wildcard * may be at beginning or end of name but not both\n\n");
    wraptext("examples: DSK1.*BAS, DSK1.MUNCHM*, TELN*, *S\n");
    wraptext("path - destination device and directory\n");
  } else if (matchcmd(tok, "delete")) {
    wraptext("==Delete File==\n\n");
    wraptext("delete <filename>\n\n");
    wraptext("Delete file in current directory\n");
  } else if (matchcmd(tok, "dir")) {
    wraptext("==List Directory==\n\n");
    wraptext("dir [/w] [path]\n\n");
    wraptext("List current directory or directory for path\n\n");
    wraptext("/w - optional. Output a simplified listing in multiple columns\n\n");
    wraptext("path - full or relative path to directory or file pattern. Wildcard * may be at beginning or end of name but not both\n\n");
    wraptext("examples: DSK1.*BAS, DSK1.MUNCHM*, TELN*, *S\n");
  } else if (matchcmd(tok, "drives")) {
    wraptext("==List Drives==\n\n");
    wraptext("drives\n\n");
    wraptext("List device names grouped by CRU base address\n");
  } else if (matchcmd(tok, "echo")) {
    wraptext("==Echo to Terminal==\n\n");
    wraptext("echo [/n] [text]\n\n");
    wraptext("Print text to ANSI terminal screen\n\n");
    wraptext("/n : optional, do not print newline after text\n\n");
    wraptext("substitutions: \\e will output an ESCAPE, ascii 27 character\n");
  } else if (matchcmd(tok, "env")) {
    wraptext("==Environment Variables==\n\n");
    wraptext("env\n\n");
    wraptext("List all variables and their values\n\n");
    wraptext("Variables are set by assigning a name of up to 10 characters a value, or by specific commands.\n\n");
    wraptext("example: NAME1=FLINSTONE\n");
    wraptext("Variable names are case insensitive.\n");
    wraptext("Variables may be used in any command by the '$' + <name> format.\n\n");
    wraptext("You may disambiguate with the '$' '(' <name ')' syntax.\n\n");
    wraptext("examples:\n");
    wraptext("echo $NAME1\n");
    wraptext("echo $(NAME1)A99\n");
  } else if (matchcmd(tok, "exit")) {
    wraptext("==Exit Force Command==\n\n");
    wraptext("exit\n\n");
    wraptext("Quit Force Command. If you are running in the auto start mode, Force Command will restart\n");
  } else if (matchcmd(tok, "fg99")) {
    wraptext("==FinalGROM==\n\n");
    wraptext("fg99 <cart>\n\n");
    wraptext("Load cartridge from FinalGROM99 sd card\n\n");
    wraptext("cart - maximum 8 character name without the '.bin'\n\n");
    wraptext("Cart must be in the same directory on the sd card as Force Command\n");
  } else if (matchcmd(tok, "ftp")) {
    wraptext("==FTP Client==\n\n");
    wraptext("ftp\n\n");
    wraptext("Open an ftp prompt. Data connection will be in passive mode only.\n\n");
    wraptext("Requires TIPI\n");
  } else if (matchcmd(tok, "goto")) {
    wraptext("==Goto==\n\n");
    wraptext("goto <label>\n\n");
    wraptext("Jump to the line following a label\n");
  } else if (matchcmd(tok, "help")) {
    wraptext("==Help==\n\n");
    wraptext("help <command>\n\n");
    wraptext("Show help for individual command. Command names and options are case insensitive. Device, file and directory name or path parameters are case sensitive.\n");
  } else if (matchcmd(tok, "if")) {
    wraptext("==Conditional==\n\n");
    wraptext("if <expr> then <command>\n\n");
    wraptext("Expression following 'if' is evaluated to true or false. If the expression evaluates to true, the rest of the command is executed.\n");
    wraptext("example:\n\n");
    wraptext("if $A == C then goto LC\n\n");
    wraptext("expressions syntax:\n");
    wraptext("[not] <lvalue> ' ' <operator> ' ' <rvalue>\n\n");
    wraptext("lvalue - quoted string which may contain variable references\n");
    wraptext("rvalue - quoted string which may contain variable references\n");
    wraptext("operator - '=='\n");
  } else if (matchcmd(tok,"label")) {
    wraptext("==Label==\n\n");
    wraptext("<label-name>:\n\n");
    wraptext("A custom identifier followed by a colon defines a label that may be branched to using 'goto'\n");
  } else if (matchcmd(tok,"load")) {
    wraptext("==Load EA5 Program==\n\n");
    wraptext("load <filepath>\n\n");
    wraptext("Load and run an EA5 program image file or files\n");
  } else if (matchcmd(tok,"lvl2")) {
    wraptext("==List IO Subroutines==\n\n");
    wraptext("lvl2 <crubase>\n\n");
    wraptext("List level 2 I/O subprograms in DSR ROM at the specified CRU base address\n");
  } else if (matchcmd(tok,"mkdir")) {
    wraptext("==Make Directory==\n\n");
    wraptext("mkdir <dirname>\n\n");
    wraptext("Create new directory at the current location\n");
  } else if (matchcmd(tok,"protect")) {
    wraptext("==Protect File==\n\n");
    wraptext("protect <filename>\n\n");
    wraptext("Set protect flag for a file at the current location\n");
  } else if (matchcmd(tok,"readkey")) {
    wraptext("==Read Keyboard==\n\n");
    wraptext("readkey <varname>\n\n");
    wraptext("Set varname to value of key currently pressed\n");
  } else if (matchcmd(tok,"rename")) {
    wraptext("==Rename File==\n\n");
    wraptext("rename <old-name> <new-name>\n\n");
    wraptext("Rename a file or directory at the current location\n");
  } else if (matchcmd(tok,"rmdir")) {
    wraptext("==Remove Directory==\n\n");
    wraptext("rmdir <dirname>\n\n");
    wraptext("Remove a child directory from the current directory\n");
  } else if (matchcmd(tok,"tipibeeps")) {
    wraptext("==Play TIPI Beeps==\n\n");
    wraptext("tipibeeps\n\n");
    wraptext("Play dih-dah-dih sound list that used to be played by TIPI powerup routine\n");
  } else if (matchcmd(tok,"tipihalt")) {
    wraptext("==Halt TIPI Raspberry PI==\n\n");
    wraptext("tipihalt\n\n");
    wraptext("Shutdown Raspberry PI operating system\n");
  } else if (matchcmd(tok,"tipimap")) {
    wraptext("==TIPI DSK Mapping==\n\n");
    wraptext("tipimap auto [on|off]\n");
    wraptext("tipimap [/c] <drive> [path]\n\n");
    wraptext("Set or get a tipi drive mapping. If no drive is specified, all configuration items are listed\n");
    wraptext("/c - optional. unmap drive\n");
    wraptext("drive - the name of the drive\n");
    wraptext("path - the directory to set\n");
    wraptext("auto - set to on or off to map DSK1 when PROGRAM image is loaded\n");
  } else if (matchcmd(tok,"tipireboot")) {
    wraptext("==Reboot TIPI Raspberry PI==\n\n");
    wraptext("tipireboot\n\n");
    wraptext("Reboot Raspberry PI operating system\n");
  } else if (matchcmd(tok,"type")) {
    wraptext("==Type File to Terminal==\n\n");
    wraptext("type [/ansi] <filepath>\n\n");
    wraptext("Copy the contents of the file to the screen\n\n");
    wraptext("/ansi - optional. Open in DISPLAY/FIXED mode instead of DISPLAY/VARIABLE and terminate output on CTRL-Z character (26)\n");
  } else if (matchcmd(tok,"unprotect")) {
    wraptext("==Unprotect File==\n\n");
    wraptext("unprotect <filename>\n\n");
    wraptext("Unprotect a file in current path\n");
  } else if (matchcmd(tok,"ver")) {
    wraptext("==Print Version==\n\n");
    wraptext("ver\n\n");
    wraptext("Display Force Command version information\n");
  } else if (matchcmd(tok,"width")) {
    wraptext("==Set Screen Width==\n\n");
    wraptext("width <40|80>\n\n");
    wraptext("Change display to 40x24 or 80x30 mode. 80 column mode requires F18A video enhancement\n");
  } else {
    wraptext("no help for command: ");
    tputs_ram(tok);
    tputc('\n');
  }
  tputc('\n');
}
