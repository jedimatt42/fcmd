#include "banks.h"
#define MYBANK BANK(0)

void* memset(void *s, int c, int len) {
    unsigned char *dst = s;
    while (len > 0) {
        *dst = (unsigned char) c;
        dst++;
        len--;
    }
    return s;
}

