#ifndef _COMMANDS_H
#define _COMMANDS_H 1

// in bank 0
void handleCall();
void handleCls();
void handleExit();
void handleFg99();
void handleLoad();
void handleWidth();

// in bank 2
void handleCd();
void handleChecksum();
void handleCopy();
void handleDir();
void handleMkdir();
void handleProtect();
void handleUnprotect();

// in bank 3
void handleColor();
void handleDelete();
void handleLvl2();
void handleRename();
void handleRmdir();
void handleTipiHalt();
void handleTipiReboot();
void handleXb();

// in bank 4
void handleEcho();
void handleEnv();
void handleDrives();
void handleGoto();
void handleIf();
void handleReadkey();
void handleType();

// in bank 5
void handleHelp();

// in bank 6
void handleCFMount();
void handleTipimap();

// in bank 7
void handleFtp();


#include "banking.h"

DECLARE_BANKED_VOID(handleCopy, BANK(2), bk_handleCopy, (), ())
DECLARE_BANKED_VOID(handleDir, BANK(2), bk_handleDir, (), ())
DECLARE_BANKED_VOID(handleMkdir, BANK(2), bk_handleMkdir, (), ())
DECLARE_BANKED_VOID(handleProtect, BANK(2), bk_handleProtect, (), ())
DECLARE_BANKED_VOID(handleUnprotect, BANK(2), bk_handleUnprotect, (), ())

DECLARE_BANKED_VOID(handleColor, BANK(3), bk_handleColor, (), ())
DECLARE_BANKED_VOID(handleDelete, BANK(3), bk_handleDelete, (), ())
DECLARE_BANKED_VOID(handleLvl2, BANK(3), bk_handleLvl2, (), ())
DECLARE_BANKED_VOID(handleRename, BANK(3), bk_handleRename, (), ())
DECLARE_BANKED_VOID(handleRmdir, BANK(3), bk_handleRmdir, (), ())
DECLARE_BANKED_VOID(handleTipiHalt, BANK(3), bk_handleTipiHalt, (), ())
DECLARE_BANKED_VOID(handleTipiReboot, BANK(3), bk_handleTipiReboot, (), ())
DECLARE_BANKED_VOID(handleXb, BANK(3), bk_handleXb, (), ())

DECLARE_BANKED_VOID(handleCd, BANK(4), bk_handleCd, (), ())
DECLARE_BANKED_VOID(handleChecksum, BANK(4), bk_handleChecksum, (), ())
DECLARE_BANKED_VOID(handleEcho, BANK(4), bk_handleEcho, (), ())
DECLARE_BANKED_VOID(handleEnv, BANK(4), bk_handleEnv, (), ())
DECLARE_BANKED_VOID(handleDrives, BANK(4), bk_handleDrives, (), ())
DECLARE_BANKED_VOID(handleGoto, BANK(4), bk_handleGoto, (), ())
DECLARE_BANKED_VOID(handleIf, BANK(4), bk_handleIf, (), ())
DECLARE_BANKED_VOID(handleReadkey, BANK(4), bk_handleReadkey, (), ())
DECLARE_BANKED_VOID(handleType, BANK(4), bk_handleType, (), ())

DECLARE_BANKED_VOID(handleHelp, BANK(5), bk_handleHelp, (), ())

DECLARE_BANKED_VOID(handleCFMount, BANK(6), bk_handleCFMount, (), ())
DECLARE_BANKED_VOID(handleTipimap, BANK(6), bk_handleTipimap, (), ())

DECLARE_BANKED_VOID(handleFtp, BANK(7), bk_handleFtp, (), ())

#endif
