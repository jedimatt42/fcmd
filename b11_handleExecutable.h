#ifndef _HANDLE_EXECUTABLE_H
#define _HANDLE_EXECUTABLE_H 1

#include "banking.h"

int handleExecutable(char* ext);

char* str_token_next(char* dst, char* str, int delim);


DECLARE_BANKED(handleExecutable, BANK(11), int, bk_handleExecutable, (char *ext), (ext))
DECLARE_BANKED(str_token_next, BANK(11), char*, bk_token_cursor, (char* dst, char* str, int delim), (dst, str, delim))

#endif