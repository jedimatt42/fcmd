#include "banks.h"
#define MYBANK BANK(1)

#include "list.h"
#include <string.h>

// The TMS9900 cannot perform word accesses at odd addresses, so every list
// entry must start on an even boundary. Pad each record to an even size.
static int entry_size(char* cursor) {
  int length = ((struct ListEntry*)cursor)->length;
  return length + 2 + (length & 1);
}

void list_init(struct List* list, void* addr, void* ceiling) {
  int start = (int)addr;
  if (start & 1) {
    start++;
  }
  list->addr = (char*)start;
  list->end = (char*)start;
  list->ceiling = (char*)ceiling;
}

void list_remove_first(struct List* list) {
  if (list->end > list->addr) {
    int remove_len = entry_size(list->addr);
    char* src = list->addr + remove_len;
    char* dst = list->addr;
    while (src < list->end) {
      *dst++ = *src++;
    }
    list->end -= remove_len;
  }
}

char* list_find_last(struct List* list) {
  if (list->addr == list->end) {
    return 0;
  }
  char* cursor = list->addr;
  while (cursor < list->end) {
    char* next = cursor + entry_size(cursor);
    if (next >= list->end) {
      return cursor;
    }
    cursor = next;
  }
  return 0;
}

struct ListEntry* list_get(struct List* list, int index) {
  char* cursor = list->addr;
  while (index > 0 && cursor < list->end) {
    char* next = cursor + entry_size(cursor);
    if (next > list->end) {
      return 0;
    }
    cursor = next;
    index--;
  }
  if (cursor >= list->end) {
    return 0;
  }
  if (index == 0) {
    return (struct ListEntry*)cursor;
  }
  return 0;
}

void list_push(struct List* list, char* buffer, int length) {
  if (length <= 0) {
    return;
  }
  int size = length + 2 + (length & 1);
  if (list->ceiling - list->addr < size) {
    // the entry can never fit, do not discard the whole list for it
    return;
  }
  // repeatedly cut off the last entry until the new entry will fit
  while(list->ceiling - list->end < size) {
    if (list->addr == list->end) {
      // the entry is larger than the whole list
      return;
    }
    char* last = list_find_last(list);
    if (last == 0) {
      return;
    }
    list->end = last;
  }
  // now move everything from addr to end over the new record
  char* dst = list->end + size - 1;
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
  if (length & 1) {
    entry->data[length] = 0;
  }
  list->end += size;
}

void list_pop(struct List* list, char* buffer, int limit) {
  if (list->end > list->addr) {
    struct ListEntry* entry = (struct ListEntry*) list->addr;
    int cpylen = entry->length < limit ? entry->length : limit;
    memcpy(buffer, entry->data, cpylen);
    if (cpylen < limit) {
      buffer[cpylen] = 0;
    }
    list_remove_first(list);
  }
}
