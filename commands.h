#ifndef _COMMANDS_H
#define _COMMANDS_H 1

#include "banking.h"

#define DEC_COMMAND(b, x) \
  void handle##x();    \
  DECLARE_BANKED_VOID(handle##x, BANK(b), bk_handle##x, (), ())

DEC_COMMAND(0, Cls);
DEC_COMMAND(0, Exit);
DEC_COMMAND(0, Fg99);
DEC_COMMAND(0, Load);
DEC_COMMAND(0, Width);
DEC_COMMAND(1, History);
DEC_COMMAND(1, Readkey);
DEC_COMMAND(3, Bar);
DEC_COMMAND(3, Lvl2);
DEC_COMMAND(3, Mkdir);
DEC_COMMAND(3, Rename);
DEC_COMMAND(3, Rmdir);
DEC_COMMAND(3, TipiHalt);
DEC_COMMAND(3, TipiReboot);
DEC_COMMAND(4, Cd);
DEC_COMMAND(4, Checksum);
DEC_COMMAND(4, Drives);
DEC_COMMAND(4, Echo);
DEC_COMMAND(4, Env);
DEC_COMMAND(4, Goto);
DEC_COMMAND(4, If);
DEC_COMMAND(4, Protect);
DEC_COMMAND(4, Type);
DEC_COMMAND(4, Unprotect);
DEC_COMMAND(5, Date);
DEC_COMMAND(5, SysInfo);
DEC_COMMAND(6, CFMount);
DEC_COMMAND(6, Color);
DEC_COMMAND(6, Tipimap);
DEC_COMMAND(6, Xb);
DEC_COMMAND(7, Ftp);
DEC_COMMAND(9, Copy);
DEC_COMMAND(9, Delete);
DEC_COMMAND(9, Dir);
DEC_COMMAND(12, Debug);
DEC_COMMAND(15, Help);

#endif
