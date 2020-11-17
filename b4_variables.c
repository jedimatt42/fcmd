#include "banks.h"
#define MYBANK BANK(4)

#include "b4_variables.h"
#include "b4_dictionary.h"
#include "b8_terminal.h"
#include "b1_strutil.h"
#include "string.h"

void vars_set(char* name, char* value) {
  if (bk_strlen(name) > MAX_VAR_NAME) {
    tputs_rom("Error, variable name too long, limit is ");
    bk_tputs_ram(bk_uint2str(MAX_VAR_NAME));
    bk_tputc('\n');
    return;
  }

  if (value == 0 || value[0] == 0) {
    dict_remove(&system_dict, DE_TYPE_VAR, name);
    return;
  }

  if (bk_strlen(value) > MAX_VAR_VAL) {
    tputs_rom("Error, variable value too long, limit is ");
    bk_tputs_ram(bk_uint2str(MAX_VAR_VAL));
    bk_tputc('\n');
    return;
  }

  dict_put(&system_dict, DE_TYPE_VAR, name, value);
}

char* vars_get(char* name) {
  char* val = (char*) dict_get(&system_dict, DE_TYPE_VAR, name) ;
  if (val) {
    return val;
  }
  return (char*) -1;
}

int onVarEntry(struct DictEntry* entry) {
  if (entry->type == DE_TYPE_VAR) {
    bk_tputs_ram(entry->data);
    bk_tputc('=');
    bk_tputs_ram(entry->data + entry->keylen + 1);
    bk_tputc('\n');
  }
  return 0;
}

void printVars() {
  dict_visit(&system_dict, onVarEntry);
  bk_tputc('\n');
}
