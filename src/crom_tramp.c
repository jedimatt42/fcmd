#include "banking.h"
#define MYBANK BANK(0)

#if CONSOLE_ROM == BUILD_CONSOLE

__attribute__((section(".data"))) void crom_tramp() {
    __asm__(
        "LWPI >83E0\n\t"
        "LI   r1, 63\n\t"
        "MOV  r1, @>0000\n\t"
        "BL   @>0000\n\t"
        "LI   r1, 0\n\t"
        "MOV  r1, @>0000\n\t"
        "LWPI >8300\n\t"
    );
}
int api_crom_tramp = (int)crom_tramp;

#endif
