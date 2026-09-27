#include "banking.h"
#define MYBANK BANK(0)

#ifndef BASE_HEX_STR
#define BASE_HEX_STR 6000
#endif
#ifndef BASE_80_HEX_STR
#define BASE_80_HEX_STR 6080
#endif

#define XSTR(x) #x
#define STR(x) XSTR(x)

void fc_api() {
    __asm__(
        // make 4 words available on stack
        "ai r10,-8\n\t"
        // stash caller return address
        "mov r11,@6(r10)\n\t"
        // The caller places the API index in the fixed word at >2002 (trampdata).
        // Do NOT read it from r0: the C backend uses r0 as a scratch register
        // when setting up calls with 8 or more arguments, which clobbers it.
        "mov @trampdata,r0\n\t"
        // set trampdata to allocated struct on stack
        "mov r10,@trampdata\n\t"
        // r0 is the index of the api entry, multiply by 2 as each entry is 2 words
        "sla r0,2\n\t"
        // add beginning of api table
        "ai  r0,>" STR(BASE_80_HEX_STR) "\n\t"
        // copy target address into trampdata
        "mov *r0+,@4(r10)\n\t"
        // copy target bank into trampdata
        "mov *r0,*r10\n\t"
        // load return bank address for cartridge bank 0
        "li r0,>" STR(BASE_HEX_STR) "\n\t"
        // copy return bank into trampdata
        "mov r0,@2(r10)\n\t"
        // now that trampdata is setup from table, call the api through the cartridge trampoline
        "bl @trampoline\n\t"
        // restore return address from stack
        "mov @6(r10),r11\n\t"
        // restore stack
        "ai r10,8\n\t"
        :
        :
        : "r0"
    );
}
