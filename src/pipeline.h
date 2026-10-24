#ifndef _PIPELINE_H
#define _PIPELINE_H 1

#include "banking.h"

/* bank 10 - called through the trampoline from pipeline.c */
int executeStage(char* buffer, const char* forced_path, int* redirect_failed);
DECLARE_BANKED(executeStage, BANK(10), int, bk_executeStage,
               (char* buffer, const char* forced_path, int* redirect_failed),
               (buffer, forced_path, redirect_failed))

void setCommandStatus(int status);
DECLARE_BANKED_VOID(setCommandStatus, BANK(10), bk_setCommandStatus,
                    (int status), (status))

/* bank 7 - called through the trampoline from parsing.c */
int runPipeline(char* buffer);
DECLARE_BANKED(runPipeline, BANK(7), int, bk_runPipeline, (char* buffer), (buffer))

#endif
