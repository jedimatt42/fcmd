#ifndef _LVL2_H
#define _LVL2_H 1

#include "b2_dsrutil.h"

#define LVL2_OP_SECTOR 0x10
#define LVL2_OP_FORMAT 0x11
#define LVL2_OP_PROTECT 0x12
#define LVL2_OP_RENAME 0x13
#define LVL2_OP_INPUT 0x14
#define LVL2_OP_OUTPUT 0x15
#define LVL2_OP_SETDIR 0x17
#define LVL2_OP_MKDIR 0x18
#define LVL2_OP_DELDIR 0x19
#define LVL2_OP_RENDIR 0x1A

#define VDPFBUF (FBUF + 0x0100)

struct AddInfo {
  unsigned int buffer;
  unsigned int first_sector;
  unsigned char flags;
  unsigned char recs_per_sec;
  unsigned char eof_offset;
  unsigned char rec_length;
  unsigned int records; // swizzled
};

unsigned int path2iocode(const char* currentPath);

unsigned int lvl2_mkdir(int crubase, unsigned int iocode, char* dirname);
unsigned int lvl2_protect(int crubase, unsigned int iocode, char* filename, int protect);
unsigned int lvl2_rename(int crubase, unsigned int iocode, char* oldname, char* newname);
unsigned int lvl2_rendir(int crubase, unsigned int iocode, char* oldname, char* newname);
unsigned int lvl2_rmdir(int crubase, unsigned int iocode, char *dirname);
unsigned int lvl2_setdir(int crubase, unsigned int iocode, char *path);
unsigned int lvl2_input(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr);
unsigned int lvl2_output(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr);

unsigned int lvl2_sector_read(int crubase, unsigned int iocode, unsigned int sector, char* bufaddr);
unsigned int lvl2_sector_write(int crubase, unsigned int iocode, unsigned int sector, char* bufaddr);

unsigned int lvl2_format(int crubase, unsigned int iocode, unsigned int tracks, unsigned int density, unsigned int sides, unsigned int interleave);

unsigned char __attribute__((noinline)) base_lvl2(int crubase, unsigned int iocode, char operation, char* name1, char* name2, char param0);
unsigned char __attribute__((noinline)) direct_io(int crubase, unsigned int iocode, char operation, char* filename, unsigned char blockcount, struct AddInfo* addInfoPtr);
void __attribute__((noinline)) call_lvl2(int crubase, unsigned char operation);
void call_basic_sub(int crubase, char* subroutine);

#include "banking.h"

DECLARE_BANKED(path2iocode, BANK(2), unsigned int, bk_path2iocode, (const char* currentPath), (currentPath))

DECLARE_BANKED(lvl2_mkdir, BANK(2), unsigned int, bk_lvl2_mkdir, (int crubase, unsigned int iocode, char *dirname), (crubase, iocode, dirname))
DECLARE_BANKED(lvl2_setdir, BANK(2), unsigned int, bk_lvl2_setdir, (int crubase, unsigned int iocode, char* path), (crubase, iocode, path))
DECLARE_BANKED(lvl2_protect, BANK(2), unsigned int, bk_lvl2_protect, (int crubase, unsigned int iocode, char *filename, int protect), (crubase, iocode, filename, protect))
DECLARE_BANKED(lvl2_rename, BANK(2), unsigned int, bk_lvl2_rename, (int crubase, unsigned int iocode, char *oldname, char *newname), (crubase, iocode, oldname, newname))
DECLARE_BANKED(lvl2_rendir, BANK(2), unsigned int, bk_lvl2_rendir, (int crubase, unsigned int iocode, char *oldname, char *newname), (crubase, iocode, oldname, newname))
DECLARE_BANKED(lvl2_rmdir, BANK(2), unsigned int, bk_lvl2_rmdir, (int crubase, unsigned int iocode, char *dirname), (crubase, iocode, dirname))
DECLARE_BANKED(lvl2_input, BANK(2), unsigned int, bk_lvl2_input, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))
DECLARE_BANKED(lvl2_output, BANK(2), unsigned int, bk_lvl2_output, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))

DECLARE_BANKED(lvl2_sector_read, BANK(2), unsigned int, bk_lvl2_sector_read, (int crubase, unsigned int iocode, unsigned int sector, char* bufaddr), (crubase, iocode, sector, bufaddr))
DECLARE_BANKED(lvl2_sector_write, BANK(2), unsigned int, bk_lvl2_sector_write, (int crubase, unsigned int iocode, unsigned int sector, char*bufaddr), (crubase, iocode, sector, bufaddr))

DECLARE_BANKED(lvl2_format, BANK(2), unsigned int, bk_lvl2_format, (int crubase, unsigned int iocode, unsigned int tracks, unsigned int density, unsigned int sides, unsigned int interleave), (crubase, iocode, tracks, density, sides, interleave))

DECLARE_BANKED_VOID(call_basic_sub, BANK(2), bk_call_basic_sub, (int crubase, char *subroutine), (crubase, subroutine))

#endif
