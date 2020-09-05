#ifndef _PARSING_H
#define _PARSING_H 1

#include "b2_dsrutil.h"

#define PR_OPTIONAL 0x0000
#define PR_REQUIRED 0x0001
#define PR_WILDCARD 0x0002

void handleCommand(char * buffer);

void parsePathParam(char* str_in, struct DeviceServiceRoutine** dsr, char* buffer, int requirements);
int globMatches(char* filename);

#include "banking.h"

DECLARE_BANKED_VOID(handleCommand, BANK(0), bk_handleCommand, (char* buffer), (buffer))
DECLARE_BANKED_VOID(parsePathParam, BANK(0), bk_parsePathParam, (char* str_in, struct DeviceServiceRoutine** dsr, char* buffer, int requirements), (str_in, dsr, buffer, requirements))
DECLARE_BANKED(globMatches, BANK(0), int, bk_globMatches, (char* filename), (filename))

#endif