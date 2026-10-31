#include "banks.h"
#define MYBANK BANK(4)

#include "globals.h"
#include "variables.h"
#include "dictionary.h"
#include "terminal.h"
#include "string.h"

void to_upper(char* name) {
  // changes to upper case in place
  int limit = bk_strlen(name);
  for (int i=0; i<limit; i++) {
    char c = name[i];
    if (c >= 'a' && c <= 'z') {
      name[i] = c - ('a' - 'A');
    }
  }
}

void var_set(const char* name, const char* value) {
  if (name == 0) {
    return;
  }

  if (bk_strlen(name) > MAX_VAR_NAME) {
    tputs_rom("Error, variable name too long, limit is ");
    bk_tputs_ram(bk_uint2str(MAX_VAR_NAME));
    bk_tputc('\n');
    return;
  }

  char key[MAX_VAR_NAME + 1];
  bk_strcpy(key, name);
  to_upper(key);

  if (value == 0 || value[0] == 0) {
    dict_remove(&system_dict, DE_TYPE_VAR, key);
    return;
  }

  if (bk_strlen(value) > MAX_VAR_VAL) {
    tputs_rom("Error, variable value too long, limit is ");
    bk_tputs_ram(bk_uint2str(MAX_VAR_VAL));
    bk_tputc('\n');
    return;
  }

  dict_put(&system_dict, DE_TYPE_VAR, key, value);
}

int var_get(const char* name, char* buf, int limit) {
  if (buf == 0 || limit <= 0) {
    return 0;
  }
  buf[0] = 0;

  if (name == 0 || bk_strlen(name) > MAX_VAR_NAME) {
    return 0;
  }

  char key[MAX_VAR_NAME + 1];
  bk_strcpy(key, name);
  to_upper(key);

  const char* val = dict_get(&system_dict, DE_TYPE_VAR, key);
  if (val == 0) {
    return 0;
  }

  int i = 0;
  while (val[i] != 0 && i < limit - 1) {
    buf[i] = val[i];
    i++;
  }
  buf[i] = 0;
  return 1;
}

int onVarEntry(struct DictEntry* entry) {
  if (entry->type == DE_TYPE_VAR && request_break == 0) {
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
