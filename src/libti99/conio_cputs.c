#include "banks.h"
#define MYBANK BANK(8)

#include "conio.h"

void cputs(const char *s) {
    while (*s) {
        cputc(*s);
        ++s;
    }
}


