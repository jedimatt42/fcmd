#include "banks.h"
#define MYBANK BANK(8)

#include "kscan.h"
#include <banks.h>

/*
When the console rom is banked, we use the common crom_tramp in api_data
(lower expansion RAM) that handles bank switching and calling the console ROM.
*/

#if CONSOLE_ROM == BUILD_CONSOLE

#include "crom_tramp.h"

unsigned int term_kscan(unsigned int mode) {
	KSCAN_MODE = mode;
    volatile short* tramp = (volatile short*)api_crom_tramp;
	tramp[CROM_TRAMP_BL_OFFS] = 0x000E;
	tramp[CROM_TRAMP_RET_OFFS] = MYBANK;
	((void (*)(void))api_crom_tramp)();
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
