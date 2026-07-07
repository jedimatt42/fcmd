#include "banks.h"
#define MYBANK BANK(8)

#include "conio.h"

void cclear(unsigned int len) {
    for (int idx=len; idx>0; --idx) { 
        cputc(' '); 
    }
}
