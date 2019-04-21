#include "banks.h"
#define MYBANK BANK_3

#include "commands.h"
#include "b0_main.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include <string.h>
#include <conio.h>

#define HELP_COMMANDS "autocmd call cd checksum cls copy delete drives echo exit fg99 goto help label load lvl2 mkdir protect rename rmdir set tipibeeps tipimap type unprotect ver width"

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
    wraptext(HELP_COMMANDS);
    tputs("\n\n");
    return;
  }

  if (matchcmd(tok,"autocmd")) {
    wraptext("DISPLAY VARIABLE format will run AUTOCMD file as script if found at startup on first drive in system.\n");
  } else if (matchcmd(tok,"call")) {
    wraptext("call <filepath> - run a script. Must be DISPLAY VARIABLE file.\n");
  } else if (matchcmd(tok, "cd")) {
    wraptext("cd [/w] <path>|.. - switch to a different drive or directory\n");
    wraptext("  /w : optional, output a simplified listing in multiple columns\n");
  } else if (matchcmd(tok, "checksum")) {
    wraptext("checksum <file> - 16 bit checksum of a file in the current directory\n");
  } else if (matchcmd(tok, "cls")) {
    wraptext("cls - clear the screen and relocate cursor to upper left\n");
  } else if (matchcmd(tok, "copy")) {
    wraptext("copy <filename> <path> - copy a file to a different directory or device\n");
  } else if (matchcmd(tok, "delete")) {
    wraptext("delete <filename> - delete file in current directory\n");
  } else if (matchcmd(tok, "drives")) {
    wraptext("drives - list device names grouped by CRU base address\n");
  } else if (matchcmd(tok, "echo")) {
    wraptext("echo [/n] [text] - print text to screen\n");
    wraptext("  /n : optional, do not print newline after text\n");
  } else if (matchcmd(tok, "exit")) {
    wraptext("exit - quit Force Command\n");
  } else if (matchcmd(tok, "fg99")) {
    wraptext("fg99 <cart> - load cartridge from FinalGROM99 sd card\n");
    wraptext("  cart - maximum 8 character name without the '.bin'\n");
  } else if (matchcmd(tok, "goto")) {
    wraptext("goto <label> - jump to the line following a label\n");
  } else if (matchcmd(tok, "help")) {
    wraptext("help - list available commands\n");
    wraptext("help <command> - show help for individual command\n");
  } else if (matchcmd(tok,"label")) {
    wraptext("<label-name>: - a custom identifier followed by a colon defines a label that may be branched to with 'goto'\n");
  } else if (matchcmd(tok,"load")) {
    wraptext("load <file> - load an EA5 program image file or files\n");
  } else if (matchcmd(tok,"lvl2")) {
    wraptext("lvl2 <crubase> - list level 2 io subprograms in DSR ROM at the specified CRU base address\n");
  } else if (matchcmd(tok,"mkdir")) {
    wraptext("mkdir <dirname> - create new directory in current path\n");
  } else if (matchcmd(tok,"protect")) {
    wraptext("protect <filename> - protect a file in current path\n");
  } else if (matchcmd(tok,"rename")) {
    wraptext("rename <old-name> <new-name> - rename a file or directory in the current path\n");
  } else if (matchcmd(tok,"rmdir")) {
    wraptext("rmdir <dirname> - remove a child directory from the current directory\n");
  } else if (matchcmd(tok,"set")) {
    wraptext("set color <F> [B] - set text and background color (0-15)\n");
    wraptext("  F - foreground text color\n");
    wraptext("  B - optional, background color\n");
    wraptext("In 80 column mode, this sets colors for subsequent text, and current border.\n");
    wraptext("In 40 column mode, it sets the full screen colors.\n");
    wraptext("colors are TI \n");
  } else if (matchcmd(tok,"tipibeeps")) {
    wraptext("tipibeeps - play tipi styled sound list\n");
  } else if (matchcmd(tok,"tipimap")) {
    wraptext("tipimap [/c] [ auto on|off ] [ <drive> [path] ] - set or get a tipi drive mapping\n");
    wraptext("    If no drive is specified, all configuration items are listed\n");
    wraptext("  /c - optional, if specified erases the specified drive mapping\n");
    wraptext("  drive - the name of the drive\n");
    wraptext("  path - the directory to set\n");
    wraptext("  auto - set to on or off to map DSK1 when PROGRAM image is loaded\n");
  } else if (matchcmd(tok,"type")) {
    wraptext("type <filepath> - copy the contents of the file to the screen\n");
  } else if (matchcmd(tok,"unprotect")) {
    wraptext("unprotect <filename> - unprotect a file in current path\n");
  } else if (matchcmd(tok,"ver")) {
    wraptext("ver - display Force Command version information\n");
  } else if (matchcmd(tok,"width")) {
    wraptext("width <40|80> - change display to 40 or 80 column mode\n");
  } else {
    tputs("no help for command: ");
    tputs(tok);
    tputc('\n');
  }
  tputc('\n');
}
