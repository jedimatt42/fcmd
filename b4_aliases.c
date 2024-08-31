#include "banks.h"
#define MYBANK BANK(4)

#include "b4_aliases.h"
#include "b4_dictionary.h"
#include "b8_terminal.h"
#include "string.h"

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
  to_upper(name);

  char* val = (char*) dict_get(&system_dict, DE_TYPE_ALIAS, name) ;
  if (val) {
    return val;
  }
  return (char*) -1;
}
