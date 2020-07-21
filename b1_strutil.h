#ifndef _STRUTIL_H
#define _STRUTIL_H 1

#include <string.h>
#include "banking.h"

int strcmp(const char* a, const char* b);
int strcmpi(const char* a, const char* b);
int str_equals(char* a, char* b);
int indexof(const char* str, int c);
int lindexof(const char* str, int c, int start);
int str_startswith(const char* str, const char* prefix);
int str_endswith(const char* str, const char* suffix);
int basicToCstr(const char* str, char* buf);
int basic_strcmp(const char* basstr, const char* cstr);
char* strtok(char *str, int delim);
char* strtokpeek(char* str, int delim);
void strset(char* buffer, int value, int limit);
char* strcat(char* dst, const char* add);
char* strncpy(char* dest, char* src, int limit);
void strpad(char* dest, int limit, int pad);
int htoi(char* s);

extern char *lasts;

static inline void setstrtok(char *str)
{
    lasts = str;
}

// string constants in ROM cannot be passed directly.. use this utility to hoist them to ram first.
extern char str2ram_buf[128];

static inline char* str2ram(const char* str) {
    memcpy(str2ram_buf, str, 128);
    return str2ram_buf;
}

DECLARE_BANKED(strcmp, BANK(1), int, bk_strcmp, (const char *a, const char *b), (a, b))
DECLARE_BANKED(strcmpi, BANK(1), int, bk_strcmpi, (const char *a, const char *b), (a, b))
DECLARE_BANKED(str_equals, BANK(1), int, bk_str_equals, (const char *a, const char *b), (a, b))
DECLARE_BANKED(indexof, BANK(1), int, bk_indexof, (const char *str, int c), (str, c))
DECLARE_BANKED(lindexof, BANK(1), int, bk_lindexof, (const char *str, int c, int start), (str, c, start))
DECLARE_BANKED(str_startswith, BANK(1), int, bk_str_startswith, (const char *str, const char *prefix), (str, prefix))
DECLARE_BANKED(str_endswith, BANK(1), int, bk_str_endswith, (const char *str, const char *suffix), (str, suffix))
DECLARE_BANKED(basicToCstr, BANK(1), int, bk_basicToCstr, (const char *str, char *buf), (str, buf))
DECLARE_BANKED(basic_strcmp, BANK(1), int, bk_basic_strcmp, (const char *basstr, const char *cstr), (basstr, cstr))
DECLARE_BANKED(strtok, BANK(1), char *, bk_strtok, (char *str, int delim), (str, delim))
DECLARE_BANKED(strtokpeek, BANK(1), char *, bk_strtokpeek, (char *str, int delim), (str, delim))
DECLARE_BANKED_VOID(strset, BANK(1), bk_strset, (char *buffer, int value, int limit), (buffer, value, limit))
DECLARE_BANKED(strcat, BANK(1), char *, bk_strcat, (char *dst, const char *add), (dst, add))
DECLARE_BANKED(strncpy, BANK(1), char *, bk_strncpy, (char *dest, char *src, int limit), (dest, src, limit))
DECLARE_BANKED_VOID(strpad, BANK(1), bk_strpad, (char *dest, int limit, int pad), (dest, limit, pad))
DECLARE_BANKED(htoi, BANK(1), int, bk_htoi, (char* s), (s))

// from string.h
DECLARE_BANKED(strlen, BANK(1), int, bk_strlen, (const char *s), (s))
DECLARE_BANKED(atoi, BANK(1), int, bk_atoi, (char* s), (s))
DECLARE_BANKED(strcpy, BANK(1), char*, bk_strcpy, (char* d, const char* s), (d, s))
DECLARE_BANKED(uint2str, BANK(1), char*, bk_uint2str, (unsigned int x), (x))
DECLARE_BANKED(int2str, BANK(1), char*, bk_int2str, (int x), (x))
DECLARE_BANKED(uint2hex, BANK(1), char*, bk_uint2hex, (unsigned int x), (x))

#endif
