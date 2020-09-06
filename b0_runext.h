#ifndef _RUN_EXT_H
#define _RUN_EXT_H 1

#include "banking.h"

int runExtension(const char* ext);

DECLARE_BANKED(runExtension, BANK(0), int, bk_runExtension, (const char* ext), (ext))

#endif