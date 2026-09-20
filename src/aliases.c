#include "banks.h"
#define MYBANK BANK(4)

#include "aliases.h"
#include "dictionary.h"
#include "terminal.h"
#include "string.h"
#include "strutil.h"

// There is one defined in b4_variables.c, same bank 4, so safe to link to
void to_upper(char* name);


void alias_set(char* name, char* value) {
  if (bk_strlen(name) > MAX_ALIAS_NAME) {
    tputs_rom("Error, alias name too long, limit is ");
    bk_tputs_ram(bk_uint2str(MAX_ALIAS_NAME));
    bk_tputc('\n');
    return;
  }

  to_upper(name);

  if (value == 0 || value[0] == 0) {
    dict_remove(&system_dict, DE_TYPE_ALIAS, name);
    return;
  }

  if (bk_strlen(value) > MAX_ALIAS_VAL) {
    tputs_rom("Error, alias value too long, limit is ");
    bk_tputs_ram(bk_uint2str(MAX_ALIAS_VAL));
    bk_tputc('\n');
    return;
  }

  dict_put(&system_dict, DE_TYPE_ALIAS, name, value);
}

char* alias_get(char* name) {
  // upcase on a copy so we do not mutate the caller's buffer. The caller may
  // hand us a token that contains more than just the alias name (for example an
  // assignment like NAME=value), and that value must keep its case.
  char key[256];
  bk_strncpy(key, name, 255);
  to_upper(key);

  char* val = (char*) dict_get(&system_dict, DE_TYPE_ALIAS, key) ;
  if (val) {
    return val;
  }
  return (char*) -1;
}
