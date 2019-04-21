#include "banks.h"

#define MYBANK BANK_4

#include "commands.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "string.h"

#define SHIFTLEFT (strcpy(&tok[ci+1], &tok[ci+2]))

void handleEcho() {
  char* peek = strtokpeek(0, " ");
  int newline = 0 != strcmpi("/N", peek);
 
  if (!newline) {
    strtok(0, " "); // consume the optional /n
  }

  char* tok = strtok(0, 0);
  int ci = 0;
  while(tok[ci] != 0) {
    if (tok[ci] == '\\') {
      switch(tok[ci+1]) {
        case 'e': // replace with ESCAPE
          tok[ci] = 27;
          SHIFTLEFT;
          break;
        case '\\': // replace double escape with single slash
          // slash is already in place
          SHIFTLEFT;
          break;
        default:
          break;
      }
    }
    ci++;
  }
  tputs(tok);

  if (newline) {
    tputc('\n');
  }
}