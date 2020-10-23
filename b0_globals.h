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
extern int scripton;
extern int lineno;
extern int pal;
extern int vdp_type;

#endif
