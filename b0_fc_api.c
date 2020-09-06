#include "banking.h"

void fc_api() {
    __asm__(
        "ai r10,-8\n\t"
        "mov r11,@6(r10)\n\t"
        "mov r10,@trampdata\n\t"
        "mov *r0+,@4(r10)\n\t"
        "mov *r0,*r10\n\t"
        "li r0,>6000\n\t"
        "mov r0,@2(r10)\n\t"
        "bl @trampoline\n\t"
        "mov @6(r10),r11\n\t"
        "ai r10,8\n\t"
    );

    // int fc_tdata[3]; //  = {(int)bank, (int)0x6000, (int)realname};

    //trampdata = (int)fc_tdata;
    //trampoline();
}