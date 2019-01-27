#ifndef _COMMANDS_H
#define _COMMANDS_H 1

// in bank 0
void handleWidth();

// in bank 2
void handleCls();
void handleExit();
void handleLvl2();
void handleDrives();

// in bank 3
void handleHelp();

/*
void handleCd();
void handleChecksum();
void handleCopy();
void handleDelete();
void handleDir();
void handleMkdir();
void handleProtect();
void handleRename();
void handleRmdir();
void handleUnprotect();
*/

#include "banking.h"

DECLARE_BANKED_VOID(handleCls, BANK_2, bk_handleCls, (), ())
DECLARE_BANKED_VOID(handleDrives, BANK_2, bk_handleDrives, (), ())
DECLARE_BANKED_VOID(handleExit, BANK_2, bk_handleExit, (), ())
DECLARE_BANKED_VOID(handleLvl2, BANK_2, bk_handleLvl2, (), ())

DECLARE_BANKED_VOID(handleHelp, BANK_3, bk_handleHelp, (), ())

#endif