#ifndef _VDP9918_H
#define _VDP9918_H 1

#include "banking.h"

/*
 * Program all eight standard 9918 registers for the active 40-column text
 * mode. Call after setupScreen(40) has selected the mode and loaded the font.
 */
void vdp9918_init_text40();

DECLARE_BANKED_VOID(vdp9918_init_text40, BANK(3), bk_vdp9918_init_text40, (), ())

#endif
