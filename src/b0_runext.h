#ifndef _RUN_EXT_H
#define _RUN_EXT_H 1

#include "banking.h"

int runExecutable(const char* ext);

DECLARE_BANKED(runExecutable, BANK(0), int, bk_runExecutable, (const char* ext), (ext))

#endif