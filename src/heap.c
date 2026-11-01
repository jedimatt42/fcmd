#include "banks.h"
#define MYBANK BANK(0)

#include "heap.h"
#include "dictionary.h"

extern int __BSS_END;
extern char __LOWER_LIMIT;

char* heap_end = (char*)&__BSS_END;

/*
 * Lowest live heap allocation made after the dictionary was initialized,
 * or 0 when there is none. Pre-dictionary allocations (the DSR list) live
 * below the dictionary and do not constrain dictionary growth.
 */
char* heap_ceiling = 0;

char* alloc(int sz) {
  /*
   * The dictionary (environment variables and aliases) lives at the bottom of
   * the heap gap and grows upward. Never allocate inside it.
   */
  char* dict_floor = (char*) system_dict.end;
  if (dict_floor > heap_end) {
    heap_end = dict_floor;
  }

  if (heap_end + sz > &__LOWER_LIMIT) {
    return 0;
  }

  char* stack_addr;
  __asm__(
    "mov r10,%0"
    : "=r" (stack_addr)
  );

  if (stack_addr - heap_end < 512) {
    return 0;
  }

  if (heap_ceiling == 0 && system_dict.addr != 0) {
    heap_ceiling = heap_end;
  }

  void* result = heap_end;
  heap_end += sz;
  return result;
}

void free(char* addr) {
  heap_end = addr;
  if (heap_ceiling != 0 && heap_end <= heap_ceiling) {
    heap_ceiling = 0;
  }
}