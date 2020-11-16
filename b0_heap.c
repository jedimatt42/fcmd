#include "banks.h"
#define MYBANK BANK(0)

#include "b0_heap.h"

extern int __BSS_END;

char* heap_end = (char*)&__BSS_END;

char* alloc(int sz) {
  void* result = heap_end;
  heap_end += sz;
  return result;
}