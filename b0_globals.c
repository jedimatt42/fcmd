#include "banks.h"
#define MYBANK BANK_0

#include "b0_globals.h"

struct DeviceServiceRoutine* currentDsr;
char currentPath[256];
int backspace;
unsigned int displayWidth = 40;
int scripton = 0;
int lineno = 0;

