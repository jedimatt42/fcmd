#ifndef _DEBUG_VDP_H
#define _DEBUG_VDP_H 1

#include "banking.h"

/*
 * DEBUG subcommands for inspecting the VDP.
 *
 *   V [reg]  read control register(s) through F18A status register 15
 *   S [reg]  read status register(s) through R#15
 *
 * The V form never unlocks the F18A; it only reports what the currently
 * selected personality returns.
 */
void debug_vdp_command(char* tok);

DECLARE_BANKED_VOID(debug_vdp_command, BANK(2), bk_debug_vdp_command, (char* tok), (tok))

#endif
