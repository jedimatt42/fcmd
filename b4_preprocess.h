#ifndef _PREPROCESS_H
#define _PREPROCESS_H

#include "banking.h"


char* preprocess(char* buf);

DECLARE_BANKED(preprocess, BANK_4, char*, bk_preprocess, (char* buf), (buf))

#endif