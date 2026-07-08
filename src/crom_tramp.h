#ifndef _CROM_TRAMP
#define _CROM_TRAMP 1

#if CONSOLE_ROM == BUILD_CONSOLE

#define CROM_TRAMP_BL_OFFS   7
#define CROM_TRAMP_RET_OFFS  9

extern int api_crom_tramp;

#endif

#endif