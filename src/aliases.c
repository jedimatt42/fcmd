#include "banks.h"
#define MYBANK BANK(4)

#include "aliases.h"
#include "dictionary.h"
#include "terminal.h"
#include "string.h"
#include "strutil.h"

// There is one defined in b4_variables.c, same bank 4, so safe to link to
void to_upper(char* name);


void alias_set(const char* name, const char* value) {
  if (name == 0) {
    return;
  }

  if (bk_strlen(name) > MAX_ALIAS_NAME) {
    tputs_rom("Error, alias name too long, limit is ");
    bk_tputs_ram(bk_uint2str(MAX_ALIAS_NAME));
    bk_tputc('\n');
    return;
  }

  char key[MAX_ALIAS_NAME + 1];
  bk_strcpy(key, name);
  to_upper(key);

  if (value == 0 || value[0] == 0) {
    dict_remove(&system_dict, DE_TYPE_ALIAS, key);
    return;
  }

  if (bk_strlen(value) > MAX_ALIAS_VAL) {
    tputs_rom("Error, alias value too long, limit is ");
    bk_tputs_ram(bk_uint2str(MAX_ALIAS_VAL));
    bk_tputc('\n');
    return;
  }

  dict_put(&system_dict, DE_TYPE_ALIAS, key, value);
}

int alias_get(const char* name, char* buf, int limit) {
  if (buf == 0 || limit <= 0) {
    return 0;
  }
  buf[0] = 0;

  if (name == 0 || bk_strlen(name) > MAX_ALIAS_NAME) {
    return 0;
  }

  // Upcase on a copy so we do not mutate the caller's buffer. The caller may
  // hand us a token that contains more than just the alias name (for example
  // an assignment like NAME=value), and that value must keep its case.
  char key[MAX_ALIAS_NAME + 1];
  bk_strcpy(key, name);
  to_upper(key);

  const char* val = dict_get(&system_dict, DE_TYPE_ALIAS, key);
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
