#ifndef _HISTORY_H
#define _HISTORY_H 1

#include "banking.h"

#define HIST_GET 0
#define HIST_STORE 1
#define HIST_INIT 2
#define HIST_GET_DEC 3

void history_init();
void history_redo(char* buffer, int limit, int op);

DECLARE_BANKED_VOID(history_init, BANK(1), bk_history_init, (), ())
DECLARE_BANKED_VOID(history_redo, BANK(1), bk_history_redo, (char* buffer, int limit, int op), (buffer, limit, op))

#endif