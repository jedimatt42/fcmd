#ifndef _HANDLE_EXECUTABLE_H
#define _HANDLE_EXECUTABLE_H 1

#include "banking.h"

void handleExecutable(char* ext);

char* token_cursor(char* dst, char* str, int delim);


DECLARE_BANKED_VOID(handleExecutable, BANK(11), bk_handleExecutable, (char *ext), (ext))
DECLARE_BANKED(token_cursor, BANK(11), char*, bk_token_cursor, (char* dst, char* str, int delim), (dst, str, delim))

#endif