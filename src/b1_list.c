#include "banks.h"
#define MYBANK BANK(1)

#include "b1_list.h"
#include <string.h>

void list_init(struct List* list, void* addr, void* ceiling) {
  list->addr = (char*)addr;
  list->end = (char*)addr;
  list->ceiling = (char*)ceiling;
}

void list_remove_first(struct List* list) {
  if (list->end > list->addr) {
    int remove_len = ((struct ListEntry*)(list->addr))->length + 2;
    char* src = list->addr + remove_len;
    memcpy(list->addr, src, list->end - src);
    list->end -= remove_len;
  }
}

char* list_find_last(struct List* list) {
  if (list->addr != list->end) {
    char* cursor = list->addr;
    while ((((struct ListEntry*)cursor)->length) + 2 < (int)list->end) {
      cursor += ((struct ListEntry*)cursor)->length + 2;
    }
    return cursor;
  } else {
    return 0;
  }
}

struct ListEntry* list_get(struct List* list, int index) {
  char* cursor = list->addr;
  while(index && (cursor < list->end)) {
    cursor += ((struct ListEntry*) cursor)->length + 2;
    index--;
  }
  if (cursor == list->end) {
    return 0;
  }
  if (!index) {
    return (struct ListEntry*)cursor;
  }
  return 0;
}

void list_push(struct List* list, char* buffer, int length) {
  // repeatedly cut off the last entry until the new entry will fit
  while(list->ceiling - list->end < 2 + length) {
    char* last = list_find_last(list);
    list->end = last;
  }
  // now move everything from addr to end over length + 2
  char* dst = list->end + length + 1;
  char* src = list->end - 1;
  int cnt = list->end - list->addr;
  while(cnt) {
    *dst-- = *src--;
    cnt--;
  }
  // then finally copy the new entry into the head of the list
  struct ListEntry* entry = (struct ListEntry*) list->addr;
  entry->length = length;
  memcpy(entry->data, buffer, length);
  list->end += length + 2;
}

void list_pop(struct List* list, char* buffer, int limit) {
  if (list->end > list->addr) {
    struct ListEntry* entry = (struct ListEntry*) list->addr;
    int cpylen = entry->length < limit ? entry->length : limit;
    memcpy(buffer, entry->data, cpylen);
    list_remove_first(list);
  }
}
