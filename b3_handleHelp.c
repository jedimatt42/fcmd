#include "banks.h"
#define MYBANK BANK_3

#include "commands.h"
#include "b0_main.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include <string.h>
#include <conio.h>

#define HELP_COMMANDS "cd checksum cls copy delete drives echo exit fg99 help load lvl2 mkdir protect rename rmdir tipibeeps tipimap unprotect ver width"

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
        cputc('\n');
      } else {
        cputc(str[i]);
      }
    } else {
      cputc(str[i]);
    }
    i++;
  }
}

void handleHelp() {
  char* tok = strtok(0, " ");
  if (tok == 0) {
    wraptext(HELP_COMMANDS);
    cputs("\n\n");
    return;
  }

  if (matchcmd(tok, "cd")) {
    cputs("cd [/w] <path>|.. - switch to a different drive or directory\n");
    cputs("  /w : optional, output a simplified listing in multiple columns\n");
  } else if (matchcmd(tok, "checksum")) {
    cputs("checksum <file> - 16 bit checksum of a file in the current directory\n");
  } else if (matchcmd(tok, "cls")) {
    cputs("cls - clear the screen and relocate cursor to upper left\n");
  } else if (matchcmd(tok, "copy")) {
    cputs("copy <filename> <path> - copy a file to a different directory or device\n");
  } else if (matchcmd(tok, "delete")) {
    cputs("delete <filename> - delete file in current directory\n");
  } else if (matchcmd(tok, "drives")) {
    cputs("drives - list device names grouped by CRU base address\n");
  } else if (matchcmd(tok, "echo")) {
    cputs("echo [/n] [text] - print text to screen\n");
    cputs("  /n : optional, do not print newline after text\n");
  } else if (matchcmd(tok, "exit")) {
    cputs("exit - quit TIPICMD\n");
  } else if (matchcmd(tok, "fg99")) {
    cputs("fg99 <cart> - load cartridge from FinalGROM99 sd card\n");
    cputs("  cart - maximum 8 character name without the '.bin'\n");
  } else if (matchcmd(tok, "help")) {
    cputs("help - list available commands\n");
    cputs("help <command> - show help for individual command\n");
  } else if (matchcmd(tok,"load")) {
    cputs("load <file> - load an EA5 program image file or files\n");
  } else if (matchcmd(tok,"lvl2")) {
    cputs("lvl2 <crubase> - list level 2 io subprograms in DSR ROM at the specified CRU base address\n");
  } else if (matchcmd(tok,"mkdir")) {
    cputs("mkdir <dirname> - create new directory in current path\n");
  } else if (matchcmd(tok,"protect")) {
    cputs("protect <filename> - protect a file in current path\n");
  } else if (matchcmd(tok,"rename")) {
    cputs("rename <old-name> <new-name> - rename a file or directory in the current path\n");
  } else if (matchcmd(tok,"rmdir")) {
    cputs("rmdir <dirname> - remove a child directory from the current directory\n");
  } else if (matchcmd(tok,"tipibeeps")) {
    cputs("tipibeeps - play tipi styled sound list\n");
  } else if (matchcmd(tok,"tipimap")) {
    cputs("tipimap [/c] [ <drive> [path] ] - set or get a tipi drive mapping\n");
    cputs("    If no drive is specified, all configuration items are listed\n");
    cputs("  /c - optional, if specified erases the specified drive mapping\n");
    cputs("  drive - the name of the drive\n");
    cputs("  path - the directory to set\n");
  } else if (matchcmd(tok,"unprotect")) {
    cputs("unprotect <filename> - unprotect a file in current path\n");
  } else if (matchcmd(tok,"ver")) {
    cputs("ver - display TIPICMD version information\n");
  } else if (matchcmd(tok,"width")) {
    cputs("width <40|80> - change display to 40 or 80 column mode\n");
  } else {
    cputs("no help for command: ");
    cputs(tok);
    cputc('\n');
  }
  cputc('\n');
}
