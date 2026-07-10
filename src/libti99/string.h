// String code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#ifndef STRING_H
#define STRING_H

#include <banking.h>

// str_len - returns the length of a zero terminated string
int str_len(const char *s);

// converts a string to an integer. No overflow detection, ignores
// leading whitespace, and converts to the first non-digit character.
int str_to_int(char *s);

// copy a zero-terminated string from s to d, returns d
char *str_copy(char *d, const char *s);

// copy a block of memory from src to dest, of cnt bytes. Returns dest.
void *memcpy(void *dest, const void *src, int cnt);

// returns a pointer to a static string, a number converted as unsigned
// Not thread safe, don't use from interrupt handlers.
char *str_from_uint(unsigned int x);

// returns a pointer to a static hex string, a number converted as unsigned
// Not thread safe, don't use from interrupt handlers.
char* hex_from_uint(unsigned int x);

// bank switchable routines
DECLARE_BANKED(str_len, BANK(1), int, bk_strlen, (const char *s), (s))
DECLARE_BANKED(str_to_int, BANK(1), int, bk_atoi, (char *s), (s))
DECLARE_BANKED(str_copy, BANK(1), char *, bk_strcpy, (char *d, const char *s), (d, s))
DECLARE_BANKED(str_from_uint, BANK(1), char *, bk_uint2str, (unsigned int x), (x))
DECLARE_BANKED(hex_from_uint, BANK(1), char *, bk_uint2hex, (unsigned int x), (x))

#endif /* STRING_H */
