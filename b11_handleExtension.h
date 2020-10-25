#ifndef _HANDLE_EXTENSION_H
#define _HANDLE_EXTENSION_H 1

#include "banking.h"

void handleExtension(char* ext);

DECLARE_BANKED_VOID(handleExtension, BANK(11), bk_handleExtension, (char *ext), (ext))


#endif