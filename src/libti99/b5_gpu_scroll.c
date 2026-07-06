#include "banks.h"
#define MYBANK BANK(5)

#include "b5_gpu_scroll.h"
#include <vdp.h>

static void gpu_scroll(void) {
    __asm__(
        // copy code starting at loop0 to VDP address >4000
        "   li r0,>3f16\n"  // source
        "	li r1,>4000\n"   // dest
        "   li r2,>34/2\n" // count
        "loopx%=\n"
        "	mov *r0+,*r1+\n"
        "	dec r2\n"
        "   jne loopx%=\n"
        "   b @>4000\n"

        // move 29 rows of chars up
        "loop0%=\n"
        "	li r0,80\n"
        "	clr r1\n"
        "	li r2,80*29/2\n"
        "loop1%=\n"
        "	mov *r0+,*r1+\n"
        "	dec r2\n"
        "	jne loop1%=\n"
        // fill bottom row with spaces
        "	li r0,>2020\n"
        "   li r2,80/2\n"
        "loop1a%=\n"
        "	mov r0,*r1+\n"
        "	dec r2\n"
        "	jne loop1a%=\n"
        // move 23 rows of colors up
        "	li r0,0x1800+80\n"
        "	li r1,0x1800\n"
        "	li r2,80*29/2\n"
        "loop2%=\n"
        "	mov *r0+,*r1+\n"
        "	dec r2\n"
        "	jne loop2%=\n"
        // stop gpu and restart when triggered
        "	idle\n"
        "	jmp loop0%=\n"
        :::
    "r0", "r1", "r2"
        );
}

static void title_gpu_scroll(void) {
    __asm__(
        // copy code starting at loop0 to VDP address >4000
        "   li r0,>3f16\n"  // source
        "	li r1,>4000\n"   // dest
        "   li r2,>36/2\n" // count
        "loopx%=\n"
        "	mov *r0+,*r1+\n"
        "	dec r2\n"
        "   jne loopx%=\n"
        "   b @>4000\n"

        // move 28 rows of chars up
        "loop0%=\n"
        "	li r0,160\n"
        "	li r1,80\n"
        "	li r2,80*28/2\n"
        "loop1%=\n"
        "	mov *r0+,*r1+\n"
        "	dec r2\n"
        "	jne loop1%=\n"
        // fill bottom row with spaces
        "	li r0,>2020\n"
        "   li r2,80/2\n"
        "loop1a%=\n"
        "	mov r0,*r1+\n"
        "	dec r2\n"
        "	jne loop1a%=\n"
        // move 28 rows of colors up
        "	li r0,0x1800+160\n"
        "	li r1,0x1800+80\n"
        "	li r2,80*28/2\n"
        "loop2%=\n"
        "	mov *r0+,*r1+\n"
        "	dec r2\n"
        "	jne loop2%=\n"
        // stop gpu and restart when triggered
        "	idle\n"
        "	jmp loop0%=\n"
        :::
    "r0", "r1", "r2"
        );
}

void install_gpu_scroll() {
    if (nTitleLine) {
        vdpmemcpy(0x3f00, (unsigned char*)title_gpu_scroll, 0x4C);
    } else {
        vdpmemcpy(0x3f00, (unsigned char*)gpu_scroll, 0x4a);
    }
}