#ifndef _DICTIONARY_H
#define _DICTIONARY_H 1

#include "banking.h"

#define DE_TYPE_VAR 1
#define DE_TYPE_LABEL 2

struct DictEntry {
    char type;
    char keylen;
    char vallen;
    char data[];
};

struct Dictionary {
    struct DictEntry* addr;
    struct DictEntry* end;
};

extern struct Dictionary system_dict;

void dict_init(struct Dictionary* dict, char* addr);
int dict_put(struct Dictionary* dict, int type, const char* key, const char* value);
const char* dict_get(struct Dictionary* dict, int type, const char* key);
const char* dict_getncpy(struct Dictionary* dict, int type, const char* key, char* buf, int limit);
int dict_remove(struct Dictionary* dict, int type, const char* key);

typedef int (*dict_visitor_cb)(struct DictEntry*);
void dict_visit(struct Dictionary* dict, dict_visitor_cb cb);

DECLARE_BANKED_VOID(dict_init, BANK(4), bk_dict_init, (struct Dictionary* dict, void* addr), (dict, addr))

#endif