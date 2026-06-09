#ifndef _GLOBALS_H
#define _GLOBALS_H 1

#include "b2_dsrutil.h"

extern struct DeviceServiceRoutine* currentDsr;
extern struct DeviceServiceRoutine* scriptDsr;
extern struct PAB* scriptPab;
extern char currentPath[256];
extern char filterglob[12];
extern int backspace;
extern unsigned int displayWidth;
extern unsigned int displayHeight;
extern int scripton;
extern int pal;
extern int api_exec;
extern int request_break;
extern int* goto_line_ref;

#endif
