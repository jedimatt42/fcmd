#include "banks.h"
#define MYBANK BANK_1

#include "b1_libtoram.h"
#include <string.h>

void libtoram() {
  extern int* __LIB_COPY;  // rom location to copy from.
  extern int __STATS_LIBTI99; // size of library code to copy to RAM.
  extern int* __LIB_START; // where the library code should end up. 
  
  memcpy(&__LIB_START, &__LIB_COPY, (int) &__STATS_LIBTI99);
}


