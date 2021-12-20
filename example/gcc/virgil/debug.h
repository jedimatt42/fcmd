#pragma once

#include <ioports.h>

inline void debug(int a) {
  vdp_memcpy(0, (char*) &a, 2);
  while(1) {
  }
}


