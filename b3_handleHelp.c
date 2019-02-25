#include "banks.h"
#define MYBANK BANK_3

#include "b0_main.h"
#include "b1cp_strutil.h"
#include <conio.h>

#define HELP_COMMANDS "cd checksum cls copy drives exit help lvl2 tipibeeps ver width\n\n"

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

void handleHelp() {
  char* tok = strtok(0, " ");
  if (tok == 0) {
    cputs(HELP_COMMANDS);
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
  } else if (matchcmd(tok, "drives")) {
    cputs("drives - list device names grouped by CRU base address\n");
  } else if (matchcmd(tok, "exit")) {
    cputs("exit - quit TIPICMD\n");
  } else if (matchcmd(tok, "help")) {
    cputs("help - list available commands\n");
    cputs("help <command> - show help for individual command\n");
  } else if (matchcmd(tok,"lvl2")) {
    cputs("lvl2 <crubase> - list level 2 io subprograms in DSR ROM at the specified CRU base address\n");
  } else if (matchcmd(tok,"tipibeeps")) {
    cputs("tipibeeps - play tipi styled sound list\n");
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
