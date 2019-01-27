#include "banks.h"
#define MYBANK BANK_3

#include "b0_main.h"
#include "b0_strutil.h"
#include "b1_libti99.h"

#define HELP_COMMANDS "help tipibeeps"

int matchcmd(char* input, char* exp) {
  char stackstr[80];
  int len = 0;
  while(exp[len] != 0) {
    stackstr[len] = exp[len];
    len++;
  }
  stackstr[len] = 0;
  return 0 == bk_strcmpi(input, stackstr);
}

void handleHelp() {
  char* tok = bk_strtok(0, " ");
  if (tok == 0) {
    CCPUTS(HELP_COMMANDS)
    return;
  }

  if (matchcmd(tok, "help")) {
    CCPUTS("help - list available commands\n");
    CCPUTS("help <command> - show help for individual command\n");
  } else if (matchcmd(tok,"tipibeeps")) {
    CCPUTS("tipibeeps - play tipi styled sound list\n");
  } else {
    CCPUTS("no help for command: ");
    bk_cputs(tok);
    bk_cputc('\n');
  }
}
