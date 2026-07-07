#include "banks.h"
#define MYBANK BANK(4)

#include "commands.h"
#include "strutil.h"
#include "aliases.h"
#include "dictionary.h"
#include "globals.h"
#include "terminal.h"

int onAliasEntry(struct DictEntry* entry) {
  if (entry->type == DE_TYPE_ALIAS && request_break == 0) {
    bk_tputs_ram(entry->data);
    tputs_rom(": ");
    bk_tputs_ram(entry->data + entry->keylen + 1);
    bk_tputc('\n');
  }

  return 0;
  return 0;
}

int handleAlias() {
  char* name = bk_strtokpeek(0, ' ');
  if (name) {
    // consume name
    bk_strtok(0, ' ');
    char* value = bk_strtok(0, 0);
    // if there is no value, it will unset the alias
    alias_set(name, value);
  } else {
    // if no parameters were provided, list all aliases
    dict_visit(&system_dict, onAliasEntry);
    bk_tputc('\n');
  }
  return 0;
}