#ifndef _PARSING_H
#define _PARSING_H 1

#include "b2_dsrutil.h"

#define PR_OPTIONAL 0x0000
#define PR_REQUIRED 0x0001

void handleCommand(char * buffer);

int parsePath(char* path, char* devicename);
void parsePathParam(struct DeviceServiceRoutine** dsr, char* buffer, int requirements);

#include "banking.h"

DECLARE_BANKED_VOID(parsePathParam, BANK_0, bk_parsePathParam, (struct DeviceServiceRoutine** dsr, char* buffer, int requirements), (dsr, buffer, requirements))

#endif