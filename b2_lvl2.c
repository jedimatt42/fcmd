#include "banks.h"
#define MYBANK BANK_2

#include "b2_lvl2.h"
#include "b2_dsrutil.h"
#include "b2_mds_dsrlnk.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"

#include <string.h>
#include <vdp.h>

#define GPLWSR9 *((volatile unsigned int *)0x83F2)
#define GPLWSR12 *((volatile unsigned int *)0x83F8)
#define LVL2_CRULST *((volatile unsigned int *)0x83D0)
#define LVL2_SADDR *((volatile unsigned int *)0x83D2)

#define LVL2_STATUS *((volatile unsigned char *)0x8350)
#define LVL2_UNIT *((volatile unsigned char*)0x834C)
#define LVL2_PROTECT *((volatile unsigned char*)0x834D)
#define LVL2_PARAMADDR1 *((volatile unsigned int*)0x834E)
#define LVL2_PARAMADDR2 *((volatile unsigned int*)0x8350)

#define UNITNO(x) (unsigned char)(x >> 8 & 0xFF)
#define OPNAME(x,y) (unsigned char)((x & 0x00F0)|(y & 0x00F))

// Returns lvl2 base code in LSB, and unit number in MSB
// 	Floppy disk controllers:	DSK	>1x
//	Myarc harddisk controller:	WDS	>2x
//	Scuzzy controller		SCS	>2x
//	IDE controller:			IDE	>8x
//	Ti to PC serial connection:	HDX	>9x
unsigned int path2unitmask(char *dirpath) {
  unsigned int operationSet = 0x0010;
  char drive[9];
  strncpy(drive, dirpath, 8);
  int l = indexof(drive, '.');
  drive[l] = 0;
  if (str_equals("TIPI", drive)) {
    // unit 0x00 operation set 0x10
    return 0x0010;
  }
  l--;
  char offset = '0';
  unsigned char unit = (drive[l] - offset) & 0xFF;

  drive[l] = 0;

  if (str_equals(drive, "WDS")) {
    operationSet = 0x0020;
  } else if (str_equals(drive, "SCS")) {
    operationSet = 0x0020; // yep, same as Myarc
  } else if (str_equals(drive, "IDE")) {
    operationSet = 0x0080;
  } else if (str_equals(drive, "HDX")) {
    operationSet = 0x0090;
  }

  return operationSet | (unit << 8);
}

unsigned int lvl2_protect(int crubase, unsigned int unit, char* filename, int protect) {
  return base_lvl2(crubase, unit, LVL2_OP_PROTECT, filename, 0, protect ? 0xff : 0x00);
}

unsigned int lvl2_setdir(int crubase, unsigned int unit, char* path) {
  int len = strlen(path);
  if (len > 39) {
    return 0xFE;
  }
  LVL2_PARAMADDR1 = FBUF;
  VDP_SET_ADDRESS_WRITE(FBUF);
  VDPWD = len;
  vdpmemcpy(FBUF+1, path, len);

  LVL2_UNIT = UNITNO(unit);
  LVL2_STATUS = 0;

  call_lvl2(crubase, OPNAME(unit, LVL2_OP_SETDIR));

  return LVL2_STATUS;
}

unsigned int lvl2_mkdir(int crubase, unsigned int unit, char* dirname) {
  return base_lvl2(crubase, unit, LVL2_OP_MKDIR, dirname, 0, 0);
}

unsigned int lvl2_rmdir(int crubase, unsigned int unit, char* dirname) {
  return base_lvl2(crubase, unit, LVL2_OP_DELDIR, dirname, 0, 0);
}

unsigned int lvl2_rename(int crubase, unsigned int unit, char* oldname, char* newname) {
  return base_lvl2(crubase, unit, LVL2_OP_RENAME, newname, oldname, 0);
}

unsigned int lvl2_rendir(int crubase, unsigned int unit, char* oldname, char* newname) {
  return base_lvl2(crubase, unit, LVL2_OP_RENDIR, newname, oldname, 0);
}

unsigned int lvl2_input(int crubase, unsigned int unit, char* filename, unsigned int blockcount, struct AddInfo* addInfoPtr) {
  return direct_io(crubase, unit, LVL2_OP_INPUT, filename, blockcount, addInfoPtr);
}

unsigned int lvl2_output(int crubase, unsigned int unit, char* filename, unsigned int blockcount, struct AddInfo* addInfoPtr) {
  return direct_io(crubase, unit, LVL2_OP_OUTPUT, filename, blockcount, addInfoPtr);
}

unsigned char direct_io(int crubase, unsigned int unit, char operation, char* filename, unsigned char blockcount, struct AddInfo* addInfoPtr) {
  LVL2_PARAMADDR1 = FBUF;
  strpad(filename, 10, ' ');
  vdpmemcpy(FBUF, filename, 10);

  LVL2_UNIT = UNITNO(unit);
  LVL2_PROTECT = blockcount;
  LVL2_STATUS = ((unsigned int) addInfoPtr) - 0x8300;

  addInfoPtr->buffer = VDPFBUF; // safe from file and path name overwrites.
  unsigned char opname = OPNAME(unit, operation);
  call_lvl2(crubase, opname);

  return LVL2_STATUS;
}

// Setup parameters suitably for most lvl2 calls.
unsigned char __attribute__((noinline)) base_lvl2(int crubase, unsigned int unit, char operation, char* name1, char* name2, char param0) {
  LVL2_UNIT = UNITNO(unit);
  LVL2_PROTECT = param0;
  LVL2_PARAMADDR1 = FBUF;

  strpad(name1, 10, ' ');
  vdpmemcpy(LVL2_PARAMADDR1, name1, 10);

  if (name2 == 0) {
    LVL2_STATUS = 0;
  } else {
    LVL2_PARAMADDR2 = FBUF + 10;
    strpad(name2, 10, ' ');
    vdpmemcpy(LVL2_PARAMADDR2, name2, 10);
  }

  unsigned char opname = OPNAME(unit, operation);
  call_lvl2(crubase, OPNAME(unit, opname));

  return LVL2_STATUS;
}

void __attribute__((noinline)) call_lvl2(int crubase, unsigned char operation) {
  enableROM(crubase);
  unsigned int addr = 0;
  unsigned int link = 0;

  struct DeviceRomHeader* dsrrom = (struct DeviceRomHeader*) 0x4000;
  struct NameLink* entry = (struct NameLink*) dsrrom->basiclnk;
  unsigned int searchname = 0x0100 | operation;
  while(entry != 0) {
    unsigned int entryname = *((int*)entry->name);
    if (entryname == searchname) {
      addr = entry->routine;
      link = (unsigned int) entry;
      break;
    }
    entry = entry->next;
  }
  disableROM(crubase);

  if (addr == 0) {
    LVL2_STATUS = 0xFF;
    return;
  }

  // Setup scratchpad ram to look like traditional DSRLNK had occured
  // - console ROM dsrlnk leaves crubase at this address, too many people know about it.
  LVL2_CRULST = crubase;
  // - HRD ROS expects this to find ros extension to the subroutine name list.
  LVL2_SADDR = link;

  // - GPL WS setup so we can swap workspace and then run the routine.
  GPLWSR12 = crubase;
  GPLWSR9 = addr;

  __asm__(
    	"	lwpi 0x83e0     ; get gplws\n"
      " sbo 0           ; turn on card dsr\n"
      " bl *r9          ; call rubroutine\n"
      " nop             ; lvl2 routines never 'skip' request\n"
      " sbz 0           ; turn off card dsr\n"
      " lwpi 0x8300     ; assuming gcc workspace is here\n"
  );
}