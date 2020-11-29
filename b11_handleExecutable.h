#ifndef _HANDLE_EXECUTABLE_H
#define _HANDLE_EXECUTABLE_H 1

#include "banking.h"

void handleExecutable(char* ext);

DECLARE_BANKED_VOID(handleExecutable, BANK(11), bk_handleExecutable, (char *ext), (ext))


#endif