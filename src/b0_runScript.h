#ifndef _RUNSCRIPT_H
#define _RUNSCRIPT_H 1

#include "banking.h"
#include "b2_dsrutil.h"

int runScript(struct DeviceServiceRoutine* dsr, char* scriptName);

DECLARE_BANKED(runScript, BANK(0), int, bk_runScript, (struct DeviceServiceRoutine * dsr, char *scriptName), (dsr, scriptName))

#endif
