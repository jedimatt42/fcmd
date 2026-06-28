#include "kscan.h"

unsigned int term_kscan(unsigned int mode) {
	KSCAN_MODE = mode;
	__asm__("LWPI >83E0\n\tBL @>000E\n\tLWPI >8300");
	return KSCAN_KEY;
}
