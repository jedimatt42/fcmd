#include "banking.h"

void fc_api() {
    __asm__(
        // make 4 words available on stack
        "ai r10,-8\n\t"
        // stash caller return address
        "mov r11,@6(r10)\n\t"
        // set trampdata to allocated struct on stack
        "mov r10,@trampdata\n\t"
        // r0 is set by caller to index of api entry in api table, multiply by 2 as each entry is 2 words
        "sla r0,2\n\t"
        // add beginning of api table 
        "ai  r0,>6080\n\t"
        // copy target address into trampdata
        "mov *r0+,@4(r10)\n\t"
        // copy target bank into trampdata
        "mov *r0,*r10\n\t"
        // load return bank address for cartridge bank 0
        "li r0,>6000\n\t"
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
