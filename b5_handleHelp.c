#include "banks.h"
#define MYBANK BANK_5

#include "commands.h"
#include "b0_main.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include <string.h>
#include <conio.h>

#define HELP_COMMANDS "call cd checksum cls color copy delete drives echo env exit fg99 ftp goto help if label load lvl2 mkdir protect readkey rename rmdir tipibeeps tipimap type unprotect ver width"

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
      if (displayWidth-1 < wherex() + wordlen(str+i+1)) {
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

void handleHelp() {
  char* tok = strtok(0, " ");
  if (tok == 0) {
    wraptext("help <command>\n");
    wraptext("\nAvailable commands:\n");
    wraptext(HELP_COMMANDS);
    wraptext("\n\n");
    return;
  }

  if (matchcmd(tok,"call")) {
    wraptext("call <filepath> - run a script. Must be DISPLAY VARIABLE file.\n");
    wraptext("\nSpecial file: AUTOCMD - First drive will be scanned for existance of AUTOCMD. If present it will run.\n");
    wraptext("Some behavior changes when AUTOCMD is found:\n");
    wraptext(" * startup banner is suppressed.\n");
    wraptext(" * initial tipibeeps sound is suppressed.\n");
  } else if (matchcmd(tok, "cd")) {
    wraptext("cd <path>|.. - switch to a different drive or directory\n");
  } else if (matchcmd(tok, "checksum")) {
    wraptext("checksum <file> - 16 bit checksum of a file in the current directory\n");
  } else if (matchcmd(tok, "cls")) {
    wraptext("cls - clear the screen and relocate cursor to upper left\n");
  } else if (matchcmd(tok, "color")) {
    wraptext("color <F> [B] - set text and background color (0-15)\n");
    wraptext("  F - foreground text color\n");
    wraptext("  B - optional, background color\n");
    wraptext("In 80 column mode, this sets colors for subsequent text, and current border.\n");
    wraptext("In 40 column mode, it sets the full screen colors.\n");
    wraptext("colors are TI \n");
  } else if (matchcmd(tok, "copy")) {
    wraptext("copy <fileset> <path> - copy a file set to a different directory or device\n");
    wraptext("  <fileset> - path and file pattern for source files to copy\n");
    wraptext("              wildcard '*' may be at beginning or end of name but not both\n");
    wraptext("              examples: DSK1.*BAS, DSK1.MUNCHM*, TELN*, *S\n");
    wraptext("  <path>    - destination device and directory path\n");
  } else if (matchcmd(tok, "delete")) {
    wraptext("delete <filename> - delete file in current directory\n");
  } else if (matchcmd(tok, "dir")) {
    wraptext("dir [/w] [path] - list current directory or directory for given path\n");
    wraptext("  /w : optional. Output a simplified listing in multiple columns\n");
    wraptext("  [path] - full or relative path to directory or file pattern\n");
    wraptext("           wildcard '*' may be at beginning or end of name but not both\n");
    wraptext("           examples: DSK1.*BAS, DSK1.MUNCHM*, TELN*, *S\n");
  } else if (matchcmd(tok, "drives")) {
    wraptext("drives - list device names grouped by CRU base address\n");
  } else if (matchcmd(tok, "echo")) {
    wraptext("echo [/n] [text] - print text to ANSI terminal screen\n");
    wraptext("  /n : optional, do not print newline after text\n");
    wraptext("substitutions:\n");
    wraptext(" * '\e' : will output an ESCAPE, ascii 27 character\n");
  } else if (matchcmd(tok, "env")) {
    wraptext("env - list all variables and their values\n");
    wraptext("\nVariables are set by assigning a name of up to 10 characters a value, or by specific commands.\n");
    wraptext("\nexample: NAME1=FLINSTONE\n");
    wraptext("Variable names are case insensitive.\n");
    wraptext("Variables may be used in any command by the '$' + <name> format.\n");
    wraptext("You may disambiguate with the '$' '(' <name ')' syntax.\n");
    wraptext("\nexamples:\n");
    wraptext("  echo $NAME1\n");
    wraptext("  echo $(NAME1)A99\n");
  } else if (matchcmd(tok, "exit")) {
    wraptext("exit - quit Force Command\n");
  } else if (matchcmd(tok, "fg99")) {
    wraptext("fg99 <cart> - load cartridge from FinalGROM99 sd card\n");
    wraptext("  cart - maximum 8 character name without the '.bin'\n");
  } else if (matchcmd(tok, "ftp")) {
    wraptext("ftp - open an ftp prompt. Data connection will be in passive mode only.\n");
  } else if (matchcmd(tok, "goto")) {
    wraptext("goto <label> - jump to the line following a label\n");
  } else if (matchcmd(tok, "help")) {
    wraptext("help - list available commands\n");
    wraptext("help <command> - show help for individual command\n");
  } else if (matchcmd(tok, "if")) {
    wraptext("if <expr> then <command> - conditional statement\n");
    wraptext("expression following 'if' is evaluated. If the expression evaluates to true, the rest of the command is executed.\n");
    wraptext("example:\n");
    wraptext("  if $A == C then goto LC\n");
    wraptext("expressions syntax: [not] <lvalue> ' ' <operator> ' ' <rvalue>\n");
    wraptext(" <lvalue> - quoted string which may contain variable references\n");
    wraptext(" <rvalue> - quoted string which may contain variable references\n");
    wraptext(" <operator> - '=='\n");
  } else if (matchcmd(tok,"label")) {
    wraptext("<label-name>: - a custom identifier followed by a colon defines a label that may be branched to with 'goto'\n");
  } else if (matchcmd(tok,"load")) {
    wraptext("load <file> - load and run an EA5 program image file or files\n");
  } else if (matchcmd(tok,"lvl2")) {
    wraptext("lvl2 <crubase> - list level 2 I/O subprograms in DSR ROM at the specified CRU base address\n");
  } else if (matchcmd(tok,"mkdir")) {
    wraptext("mkdir <dirname> - create new directory in current path\n");
  } else if (matchcmd(tok,"protect")) {
    wraptext("protect <filename> - protect a file in current path\n");
  } else if (matchcmd(tok,"readkey")) {
    wraptext("readkey <varname> - set varname to value of key currently pressed\n");
  } else if (matchcmd(tok,"rename")) {
    wraptext("rename <old-name> <new-name> - rename a file or directory in the current path\n");
  } else if (matchcmd(tok,"rmdir")) {
    wraptext("rmdir <dirname> - remove a child directory from the current directory\n");
  } else if (matchcmd(tok,"tipibeeps")) {
    wraptext("tipibeeps - play tipi styled sound list\n");
  } else if (matchcmd(tok,"tipimap")) {
    wraptext("tipimap [/c] [ auto on|off ] [ <drive> [path] ] - set or get a tipi drive mapping\n");
    wraptext("    If no drive is specified, all configuration items are listed\n");
    wraptext("  /c - optional. If specified, erases the specified drive mapping\n");
    wraptext("  drive - the name of the drive\n");
    wraptext("  path - the directory to set\n");
    wraptext("  auto - set to on or off to map DSK1 when PROGRAM image is loaded\n");
  } else if (matchcmd(tok,"type")) {
    wraptext("type [/ansi] <filepath> - copy the contents of the file to the screen\n");
    wraptext("  /ansi - optional. Open in DISPLAY/FIXED mode instead of DISPLAY/VARIABLE\n");
    wraptext("          and terminate output on CTRL-Z character (26)\n");
  } else if (matchcmd(tok,"unprotect")) {
    wraptext("unprotect <filename> - unprotect a file in current path\n");
  } else if (matchcmd(tok,"ver")) {
    wraptext("ver - display Force Command version information\n");
  } else if (matchcmd(tok,"width")) {
    wraptext("width <40|80> - change display to 40 or 80 column mode\n");
  } else {
    wraptext("no help for command: ");
    tputs_ram(tok);
    tputc('\n');
  }
  tputc('\n');
}
