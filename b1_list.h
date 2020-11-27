#ifndef _LIST_H
#define _LIST_H 1

#include "banking.h"

struct __attribute__((__packed__)) List {
    char* addr;
    char* end;
    char* ceiling;
};

struct __attribute__((__packed__)) ListEntry {
    int length;
    char data[];
};

// if we add append, and shift, we can call it a list, but for now we just need stack...

void list_init(struct List* list, void* addr, void* ceiling);
void list_push(struct List* list, char* buffer, int length);
void list_pop(struct List* list, char* buffer, int limit);
struct ListEntry* list_get(struct List* list, int index);

DECLARE_BANKED_VOID(list_init, BANK(1), bk_list_init, (struct List* list, void* addr, void* ceiling), (list, addr, ceiling))
DECLARE_BANKED_VOID(list_push, BANK(1), bk_list_push, (struct List* list, char* buffer, int length), (list, buffer, length))
DECLARE_BANKED_VOID(list_pop, BANK(1), bk_list_pop, (struct List* list, char* buffer, int limit), (list, buffer, limit))
DECLARE_BANKED(list_get, BANK(1), struct ListEntry*, bk_list_get, (struct List* list, int index), (list, index))

#endif