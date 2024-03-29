#ifndef _HISTORY_H
#define _HISTORY_H 1

#include "banking.h"

#define HIST_GET 0
#define HIST_STORE 1
#define HIST_INIT 2
#define HIST_GET_DEC 3

#define SAMS_HIST_ADDR 0xB000
#define HIST_IDX *((volatile int*) SAMS_HIST_ADDR)
#define HISTORY ((struct List*) (SAMS_HIST_ADDR + 2))

void history_init();
void history_redo(char* buffer, int limit, int op);
void history_indexed(char* buffer, int limit, int idx);

DECLARE_BANKED_VOID(history_init, BANK(1), bk_history_init, (), ())
DECLARE_BANKED_VOID(history_redo, BANK(1), bk_history_redo, (char* buffer, int limit, int op), (buffer, limit, op))
DECLARE_BANKED_VOID(history_indexed, BANK(1), bk_history_indexed, (char* buffer, int limit, int idx), (buffer, limit, idx))

#endif
