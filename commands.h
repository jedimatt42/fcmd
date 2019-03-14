#ifndef _COMMANDS_H
#define _COMMANDS_H 1

// in bank 0
void handleCall();
void handleCls();
void handleDelete();
void handleDrives();
void handleEcho();
void handleExit();
void handleFg99();
void handleLoad();
void handleWidth();
void handleTipimap();
void handleType();
void handleUnprotect();
void handleRmdir();

// in bank 2
void handleCd();
void handleChecksum();
void handleCopy();
void handleDir();
void handleLvl2();
void handleMkdir();
void handleProtect();

// in bank 3
void handleHelp();
void handleSet();
void handleRename();


#include "banking.h"
DECLARE_BANKED_VOID(handleCd, BANK_2, bk_handleCd, (), ())
DECLARE_BANKED_VOID(handleChecksum, BANK_2, bk_handleChecksum, (), ())
DECLARE_BANKED_VOID(handleCopy, BANK_2, bk_handleCopy, (), ())
DECLARE_BANKED_VOID(handleDir, BANK_2, bk_handleDir, (), ())
DECLARE_BANKED_VOID(handleLvl2, BANK_2, bk_handleLvl2, (), ())
DECLARE_BANKED_VOID(handleMkdir, BANK_2, bk_handleMkdir, (), ())
DECLARE_BANKED_VOID(handleProtect, BANK_2, bk_handleProtect, (), ())

DECLARE_BANKED_VOID(handleHelp, BANK_3, bk_handleHelp, (), ())
DECLARE_BANKED_VOID(handleSet, BANK_3, bk_handleSet, (), ())
DECLARE_BANKED_VOID(handleRename, BANK_3, bk_handleRename, (), ())

#endif