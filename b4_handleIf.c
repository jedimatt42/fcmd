#include "banks.h"
#define MYBANK BANK(4)

#include "b1_strutil.h"
#include "b8_terminal.h"
#include <string.h>
#include "b0_parsing.h"

#define EQ 1

static int getTruth(int operator, char* left, char* right, int negate);

void handleIf() {
  /* syntax:
     IF [NOT] <left> == <right> THEN <command>
     // if has already been consumed
  */
  int negate = 0;
  char left[30];
  char right[30];
  int operator = 0;
  char command[100];

  char* tok = bk_strtok(0, ' ');
  if (!bk_strcmpi(str2ram("not"), tok)) {
    negate = 1;
    tok = bk_strtok(0, ' ');
  }

  if (!tok) {
    tputs_rom("Error, missing left-hand value\n");
    return;
  }

  if (bk_strlen(tok) < 30) {
    bk_strcpy(left, tok);
  } else {
    tputs_rom("Error, left of expression too long, must be less than 30\n");
    return;
  }

  tok = bk_strtok(0, ' ');

  if (bk_str_equals(str2ram("=="), tok)) {
    operator = EQ;
  } else {
    tputs_rom("Error, unsupported operator\n");
    return;
  }

  tok = bk_strtok(0, ' ');

  if (!tok) {
    tputs_rom("Error, missing right-hand value\n");
    return;
  }

  if (bk_strlen(tok) < 30) {
    bk_strcpy(right, tok);
  } else {
    tputs_rom("Error, right of expression too long, must be less than 30\n");
    return;
  }

  tok = bk_strtok(0, ' ');
  if (bk_strcmpi(str2ram("then"), tok)) {
    tputs_rom("Error, missing \'then\' in statement\n");
    return;
  }

  tok = bk_strtok(0, 0); // to end of line
  if (!tok) {
    tputs_rom("Error, missing <command> in statement\n");
    return;
  }
  bk_strcpy(command, tok);

  if (getTruth(operator, left, right, negate)) {
    bk_handleCommand(command);
  }
}

static int getTruth(int operator, char* left, char* right, int negate) {
  int opres = 0;
  if (operator == EQ) {
    opres = bk_str_equals(left, right);
  } else {
    return 0;
  }
  return negate ? !opres : opres;
}

