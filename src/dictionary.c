#include "banks.h"
#define MYBANK BANK(4)

#include "dictionary.h"
#include "strutil.h"
#include <string.h>

struct Dictionary system_dict;

extern char* heap_start;
extern char __LOWER_LIMIT;

void dict_init(struct Dictionary* dict, char* addr) {
  dict->addr = (struct DictEntry*) addr;
  dict->end = (struct DictEntry*) addr;
}

inline int dict_entry_size(struct DictEntry* entry) {
    return 5 + entry->keylen + entry->vallen;
}

struct DictEntry* dict_entry(struct Dictionary* dict, int type, const char* key) {
    struct DictEntry* cursor = dict->addr;
    while (cursor < dict->end) {
        if (cursor->type == type && bk_strcmp(key, cursor->data) == 0) {
            return cursor;
        }
        cursor = (struct DictEntry*) (((char*) cursor) + dict_entry_size(cursor));
    }
    return 0;
}

void dict_visit(struct Dictionary* dict, dict_visitor_cb cb) {
    struct DictEntry* cursor = dict->addr;
    while (cursor < dict->end) {
        if (cb(cursor)) {
            return; // if the visiting callback returns non-zero, then quit.
        }
        cursor = (struct DictEntry*)(((char*)cursor) + dict_entry_size(cursor));
    }
}

int dict_put(struct Dictionary* dict, int type, const char* key, const char* value) {
    int keylen = bk_strlen(key);
    int vallen = bk_strlen(value);
    int entry_size = 5 + keylen + vallen;

    struct DictEntry* old = dict_entry(dict, type, key);
    int old_size = old ? dict_entry_size(old) : 0;
    char* new_end = ((char*) dict->end - old_size) + entry_size;

    if (new_end > &__LOWER_LIMIT) {
        return 0;
    }
    if (heap_start != 0 && new_end > heap_start) {
        return 0;
    }

    dict_remove(dict, type, key);

    struct DictEntry* newspace = dict->end;
    newspace->type = type;
    newspace->keylen = keylen;
    newspace->vallen = vallen;
    bk_strncpy(newspace->data, (char*) key, keylen);
    bk_strncpy(newspace->data + (keylen + 1), (char*) value, vallen);
    dict->end = (struct DictEntry*) (((char*) newspace) + entry_size);
    return 1;
}

const char* dict_get(struct Dictionary* dict, int type, const char* key) {
    struct DictEntry* cursor = dict_entry(dict, type, key);
    if (cursor != 0) {
        return cursor->data + cursor->keylen + 1;
    }
    return 0;
}

const char* dict_getncpy(struct Dictionary* dict, int type, const char* key, char* buf, int limit) {
    const char* value = dict_get(dict, type, key);
    if (value != 0) {
        bk_strncpy(buf, (char*) value, limit);
        return buf;
    }
    return 0;
}

int dict_remove(struct Dictionary* dict, int type, const char* key) {
    struct DictEntry* cursor = dict_entry(dict, type, key);
    if (cursor != 0) {
        // compact the others.
        int entry_size = dict_entry_size(cursor);
        char* dst = (char*) cursor;
        char* src = dst + entry_size;
        memcpy(dst, src, ((char*)dict->end) - src);
        dict->end = (struct DictEntry*) (((char*)dict->end) - entry_size);
        bk_strset((char*) dict->end, 0, entry_size);
        return 1;
    }
    return 0;
}
