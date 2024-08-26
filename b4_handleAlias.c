#include "banks.h"

#define MYBANK BANK(4)

#include "commands.h"
#include "b1_strutil.h"
#include "b4_aliases.h"

void handleAlias() {
  char* name = bk_strtok(0, ' ');
  if (name == 0) {
    printAliases();
    return;
  }

  char* value = bk_strtok(0, 0);
  alias_set(name, value);
}