#include "banks.h"
#define MYBANK BANK(0)

#include "b0_heap.h"

extern int __BSS_END;

char* heap_end = (char*)&__BSS_END;

char* alloc(int sz) {
  char* stack_addr;
  __asm__(
    "mov r10,%0"
    : "=r" (stack_addr)
  );

  if (stack_addr - heap_end < 512) {
    return 0;
  }

  void* result = heap_end;
  heap_end += sz;
  return result;
}

void free(char* addr) {
  heap_end = addr;
}