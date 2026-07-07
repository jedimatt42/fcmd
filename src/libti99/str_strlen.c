#include "banks.h"
#define MYBANK BANK(1)

// String code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

int str_len(const char *s) {
	int n = 0;
	while (*(s++)) n++;
	return n;
}

