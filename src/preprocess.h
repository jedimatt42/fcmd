#ifndef _PREPROCESS_H
#define _PREPROCESS_H

#include "banking.h"


char* preprocess(char* buf);
char* phase2_preprocess(char* buf);

DECLARE_BANKED(preprocess, BANK(4), char*, bk_preprocess, (char* buf), (buf))
DECLARE_BANKED(phase2_preprocess, BANK(4), char*, bk_phase2_preprocess, (char* buf), (buf))

#endif