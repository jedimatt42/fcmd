#include "banks.h"
#define MYBANK BANK(8)

#include "kscan.h"
#include <banks.h>

/*
When the console rom is banked, we have to carefully craft code in RAM >8320 to
make the call and return to the FCMD bank and code. If we take advantage of using
the GPLWS, we can prepare registers.
*/

#if CONSOLE_ROM == BUILD_CONSOLE

void crom_tramp() {
    __asm__(
        "LWPI >83E0\n\t"
		"LI   r1, 63\n\t"
		"MOV  r1, @>0000\n\t"
		"BL   @>000E\n\t"
        "LI   r1, 8\n\t"
		"MOV  r1, @>0000\n\t"
		"LWPI >8300\n\t"
	);
}

unsigned int term_kscan(unsigned int mode) {
	KSCAN_MODE = mode;

    int* src = (int*)crom_tramp;
	int* end = (int*)term_kscan;
	int* dst = (int*)0x8320;
	while(src < end) {
		*dst++ = *src++;
	}

	((void (*)(void))0x8320)();

	return KSCAN_KEY;
}

#else

unsigned int term_kscan(unsigned int mode) {
	KSCAN_MODE = mode;
	__asm__(
		"LWPI >83E0\n\t"
		"BL @>000E\n\t"
		"LWPI >8300\n\t"
	);
	return KSCAN_KEY;
}

#endif
