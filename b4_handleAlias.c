#include "banks.h"
#define MYBANK BANK(4)

#include "string.h"
#include "commands.h"
#include "b1_strutil.h"
#include "b4_aliases.h"
#include "b4_dictionary.h"
#include "b0_globals.h"
#include "b8_terminal.h"

int onAliasEntry(struct DictEntry* entry) {
  // if (entry->type == DE_TYPE_ALIAS && request_break == 0) {
  //   bk_tputs_ram(entry->data);
  //   bk_tputc(':');
  //   bk_tputs_ram(entry->data + entry->keylen + 1);
  //   bk_tputc('\n');
  // }
  bk_tputc('-');
  // bk_tputs_ram(bk_uint2str((int) entry));
  bk_tputc('\n');
  return 0;
}

void handleAlias() {
  char* name = bk_strtokpeek(0, ' ');
  int list = name == 0;

  if (list) {
    dict_visit(&system_dict, onAliasEntry);
    bk_tputc('\n');
  } else {
    // consume name
    bk_strtok(0, ' ');
    char* value = bk_strtok(0, 0);
    alias_set(name, value);
  }
}