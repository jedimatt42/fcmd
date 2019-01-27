#ifndef _COMMANDS_H
#define _COMMANDS_H 1

void handleCls();
void handleExit();
void handleLvl2();
void handleWidth();

/*
void handleCd();
void handleChecksum();
void handleCopy();
void handleDelete();
void handleDir();
void handleDrives();
void handleHelp();
void handleMkdir();
void handleProtect();
void handleRename();
void handleRmdir();
void handleUnprotect();
*/

#include "banking.h"

DECLARE_BANKED_VOID(handleCls, BANK_2, bk_handleCls, (), ())
DECLARE_BANKED_VOID(handleExit, BANK_2, bk_handleExit, (), ())
DECLARE_BANKED_VOID(handleLvl2, BANK_2, bk_handleLvl2, (), ())

#endif