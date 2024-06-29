#include "banks.h"
#define MYBANK BANK(2)

#include "b2_lvl2.h"
#include "b2_dsrutil.h"
#include "b1_strutil.h"

#include <string.h>
#include <vdp.h>

#define GPLWSR9 *((volatile unsigned int *)0x83F2)
#define GPLWSR12 *((volatile unsigned int *)0x83F8)
#define LVL2_CRULST *((volatile unsigned int *)0x83D0)
#define LVL2_SADDR *((volatile unsigned int *)0x83D2)

#define LVL2_STATUS *((volatile unsigned char *)0x8350)
#define LVL2_UNIT *((volatile unsigned char*)0x834C)
#define LVL2_PROTECT *((volatile unsigned char*)0x834D)
#define LVL2_READWRITE *((volatile unsigned char*)0x834D)
#define LVL2_PARAMADDR1 *((volatile unsigned int*)0x834E)
#define LVL2_PARAMADDR2 *((volatile unsigned int*)0x8350)

#define LVL2_SEC_PER_DISK *((volatile unsigned int*)0x834A)
#define LVL2_TRACKS *((volatile unsigned char *)0x834D)
#define LVL2_FORMAT_BUFFER *((volatile unsigned int*)0x834E)
#define LVL2_DENSITY *((volatile unsigned char *)0x8350)
#define LVL2_SIDES *((volatile unsigned char *)0x8351)

#define NCOMP *((volatile unsigned int*)0x8354)
#define PPAB *((volatile unsigned int*)0x8356)

#define UNITNO(x) (unsigned char)(x >> 8 & 0xFF)
#define OPNAME(x,y) (unsigned char)((x & 0x00F0)|(y & 0x00F))

static void call_addr(int crubase, int addr, int link);
static int supportsCpuBuffers(int crubase);

// Returns lvl2 device management base code in LSB, and unit number in MSB
// 	Floppy disk controllers:	DSK	>1x
//	Myarc harddisk controller:	WDS	>2x
//	Scuzzy controller		SCS	>2x
//	IDE controller:			IDE	>8x
//	Ti to PC serial connection:	HDX	>9x
unsigned int path2iocode(const char *dirpath)
{
  unsigned int operationSet = 0x0010;
  char drive[9];
  bk_strncpy(drive, (char*)dirpath, 8);
  int l = bk_indexof(drive, '.');
  drive[l] = 0;
  if (bk_str_equals(str2ram("TIPI"), drive)) {
    // unit 0x00 operation set 0x10
    return 0x0010;
  }
  if (bk_str_equals(str2ram("RD"), drive)) {
    // unit 0x07 operation set 0x10
    return 0x0710;
  }
  // jasonACT pico cart supports catalog of DSK.
  if (bk_str_equals(str2ram("DSK"), drive)) {
    // unit 0x00 operation set 0x10
    return 0x0010;
  }
  l--;
  char offset = '0';
  unsigned char unit = (drive[l] - offset) & 0xFF;

  drive[l] = 0;

  if (bk_str_equals(drive, str2ram("WDS"))) {
    operationSet = 0x0020;
  } else if (bk_str_equals(drive, str2ram("SCS"))) {
    operationSet = 0x0020; // yep, same as Myarc
  } else if (bk_str_equals(drive, str2ram("IDE"))) {
    operationSet = 0x0080;
  } else if (bk_str_equals(drive, str2ram("HDX"))) {
    operationSet = 0x0090;
  }

  return operationSet | (unit << 8);
}

unsigned int lvl2_protect(int crubase, unsigned int iocode, char* filename, int protect) {
  return base_lvl2(crubase, iocode, LVL2_OP_PROTECT, filename, 0, protect ? 0xff : 0x00);
}

struct PathString {
  char len;
  char chars[40];
};

unsigned int lvl2_setdir(int crubase, unsigned int iocode, char* path) {
  int len = bk_strlen(path);
  if (len > 39) {
    return 0xFE;
  }
  LVL2_UNIT = UNITNO(iocode);
  LVL2_STATUS = 0;
  int useCpuBuffer = supportsCpuBuffers(crubase);
  struct PathString basicString;
  if (useCpuBuffer) {
    LVL2_UNIT = LVL2_UNIT | 0x80; // set cpu buffer bit in unit number
    basicString.len = len;
    bk_strncpy(basicString.chars, path, 40);
    LVL2_PARAMADDR1 = (int) &basicString;
  } else {
    LVL2_PARAMADDR1 = FBUF;
    VDP_SET_ADDRESS_WRITE(FBUF);
    VDPWD = len;
    vdpmemcpy(FBUF+1, path, len);
  }

  call_lvl2(crubase, OPNAME(iocode, LVL2_OP_SETDIR));

  return LVL2_STATUS;
}

unsigned int lvl2_mkdir(int crubase, unsigned int iocode, char* dirname) {
  return base_lvl2(crubase, iocode, LVL2_OP_MKDIR, dirname, 0, 0);
}

unsigned int lvl2_rmdir(int crubase, unsigned int iocode, char* dirname) {
  return base_lvl2(crubase, iocode, LVL2_OP_DELDIR, dirname, 0, 0);
}

unsigned int lvl2_rename(int crubase, unsigned int iocode, char* oldname, char* newname) {
  return base_lvl2(crubase, iocode, LVL2_OP_RENAME, newname, oldname, 0);
}

unsigned int lvl2_rendir(int crubase, unsigned int iocode, char* oldname, char* newname) {
  return base_lvl2(crubase, iocode, LVL2_OP_RENDIR, newname, oldname, 0);
}

unsigned int lvl2_input(int crubase, unsigned int iocode, char* filename, unsigned int blockcount, struct AddInfo* addInfoPtr) {
  return direct_io(crubase, iocode, LVL2_OP_INPUT, filename, blockcount, addInfoPtr);
}

unsigned int lvl2_output(int crubase, unsigned int iocode, char* filename, unsigned int blockcount, struct AddInfo* addInfoPtr) {
  return direct_io(crubase, iocode, LVL2_OP_OUTPUT, filename, blockcount, addInfoPtr);
}

unsigned char direct_io(int crubase, unsigned int iocode, char operation, char* filename, unsigned char blockcount, struct AddInfo* addInfoPtr) {
  LVL2_PARAMADDR1 = FBUF;
  bk_strpad(filename, 10, ' ');
  vdpmemcpy(FBUF, filename, 10);

  LVL2_UNIT = UNITNO(iocode);
  LVL2_PROTECT = blockcount;
  LVL2_STATUS = ((unsigned int) addInfoPtr) - 0x8300;

  addInfoPtr->buffer = VDPFBUF; // safe from file and path name overwrites.
  unsigned char opname = OPNAME(iocode, operation);
  call_lvl2(crubase, opname);

  return LVL2_STATUS;
}

unsigned int lvl2_input_cpu(int crubase, unsigned int iocode, char* filename, unsigned int blockcount, struct AddInfo* addInfoPtr) {
  int status = direct_io_cpu(crubase, iocode, LVL2_OP_INPUT, filename, blockcount, addInfoPtr);
  if (!status && !supportsCpuBuffers(crubase) && blockcount != 0) {
    // device didn't support cpu buffers, so the data ended up in VDP, copy to the requested cpu buffer
    vdpmemread(VDPFBUF, (char*) addInfoPtr->buffer, blockcount * 256);
  }
  return status;
}

unsigned int lvl2_output_cpu(int crubase, unsigned int iocode, char* filename, unsigned int blockcount, struct AddInfo* addInfoPtr) {
  if (!supportsCpuBuffers(crubase) && blockcount != 0) {
    // device doesn't support cpu buffers, so copy the call cpu buffer into VDP
    vdpmemcpy(VDPFBUF, (char*) addInfoPtr->buffer, blockcount * 256);
  }
  return direct_io_cpu(crubase, iocode, LVL2_OP_OUTPUT, filename, blockcount, addInfoPtr);
}

unsigned char direct_io_cpu(int crubase, unsigned int iocode, char operation, char* filename, unsigned char blockcount, struct AddInfo* addInfoPtr) {
  LVL2_UNIT = UNITNO(iocode);
  LVL2_PROTECT = blockcount;
  LVL2_STATUS = ((unsigned int) addInfoPtr) - 0x8300;

  char param1buf[11];
  bk_strncpy(param1buf, filename, 10);
  bk_strpad(param1buf, 10, ' ');

  char* cpuBuffer = (char*) addInfoPtr->buffer;

  int useCpuBuffer = supportsCpuBuffers(crubase);
  if (useCpuBuffer) {
    LVL2_UNIT = LVL2_UNIT | 0x80; // set cpu buffer bit in unit number
    LVL2_PARAMADDR1 = (int) param1buf;
  } else {
    LVL2_PARAMADDR1 = FBUF;
    vdpmemcpy(FBUF, param1buf, 10);
    addInfoPtr->buffer = VDPFBUF; // safe from file and path name overwrites.
  }

  unsigned char opname = OPNAME(iocode, operation);
  call_lvl2(crubase, opname);

  if (!useCpuBuffer) {
    addInfoPtr->buffer = (int) cpuBuffer;
  }

  return LVL2_STATUS;
}

unsigned int lvl2_sector_read(int crubase, unsigned int iocode, unsigned int sector, char* bufaddr) {
  LVL2_UNIT = UNITNO(iocode);
  LVL2_READWRITE = 0xff;
  LVL2_PARAMADDR2 = sector;

  int useCpuBuffer = supportsCpuBuffers(crubase);
  if (useCpuBuffer) {
    LVL2_UNIT = LVL2_UNIT | 0x80; // set cpu buffer bit in unit number
    LVL2_PARAMADDR1 = (int) bufaddr;
  } else {
    LVL2_PARAMADDR1 = FBUF;
  }
  
  unsigned char opname = OPNAME(iocode, LVL2_OP_SECTOR);
  call_lvl2(crubase, opname);

  if (!useCpuBuffer) {
    vdpmemread(FBUF, bufaddr, 256);
  }

  return LVL2_STATUS;
}


unsigned int lvl2_sector_write(int crubase, unsigned int iocode, unsigned int sector, char* bufaddr) {
  LVL2_UNIT = UNITNO(iocode);
  LVL2_READWRITE = 0x00;
  LVL2_PARAMADDR2 = sector;

  int useCpuBuffer = supportsCpuBuffers(crubase);
  if (useCpuBuffer) {
    LVL2_UNIT = LVL2_UNIT | 0x80; // set cpu buffer bit in unit number
    LVL2_PARAMADDR1 = (int) bufaddr;
  } else {
    LVL2_PARAMADDR1 = FBUF;
    vdpmemcpy(FBUF, bufaddr, 256);
  }

  unsigned char opname = OPNAME(iocode, LVL2_OP_SECTOR);
  call_lvl2(crubase, opname);

  return LVL2_STATUS;
}

unsigned int lvl2_format(int crubase, unsigned int iocode, unsigned int tracks, unsigned int density, unsigned int sides, unsigned int interleave) {
  LVL2_UNIT = UNITNO(iocode) | (((density & 2) | (sides >> 1)) << 4);
  LVL2_TRACKS = (unsigned char) tracks;
  // for HFDC the interleave can be 0, for default or specified, it fits into the density as the top 6 bits while density is limited to the bottom 2 bits.
  LVL2_DENSITY = (unsigned char) ((0x03 & density) | (interleave << 2));
  LVL2_SIDES = (unsigned char) sides;
  LVL2_FORMAT_BUFFER = FBUF;
  
  unsigned char opname = OPNAME(iocode, LVL2_OP_FORMAT);
  call_lvl2(crubase, opname);

  return LVL2_SEC_PER_DISK;
}

// lookup CPU buffer support based on crubase
static int supportsCpuBuffers(int crubase) {
  int i = 0;

  while(dsrList[i].name[0] != 0) {
    // Assume cpuSup is same for all devices on a given card/crubase
    if (dsrList[i].crubase == crubase) {
      return dsrList[i].cpuSup;
    }
    i++;
  }

  return 0;
}

// Setup parameters suitably for most lvl2 calls.
unsigned char __attribute__((noinline)) base_lvl2(int crubase, unsigned int iocode, char operation, char* name1, char* name2, char param0) {
  LVL2_UNIT = UNITNO(iocode);
  LVL2_PROTECT = param0;

  char param1buf[11];
  char param2buf[11];
  bk_strncpy(param1buf, name1, 10);
  bk_strpad(param1buf, 10, ' ');
  int useCpuBuffer = supportsCpuBuffers(crubase);
  if (useCpuBuffer) {
    LVL2_UNIT = LVL2_UNIT | 0x80; // set cpu buffer bit in unit number
    LVL2_PARAMADDR1 = (int) param1buf;
  } else {
    LVL2_PARAMADDR1 = FBUF;
    vdpmemcpy(LVL2_PARAMADDR1, param1buf, 10);
  }

  if (name2 == 0) {
    LVL2_STATUS = 0;
  } else {
    bk_strncpy(param2buf, name2, 10);
    bk_strpad(param2buf, 10, ' ');
    if (useCpuBuffer) {
      LVL2_PARAMADDR2 = (int) param2buf;
    } else {
      LVL2_PARAMADDR2 = FBUF + 10;
      vdpmemcpy(LVL2_PARAMADDR2, name2, 10);
    }
  }

  unsigned char opname = OPNAME(iocode, operation);
  call_lvl2(crubase, OPNAME(iocode, opname));

  return LVL2_STATUS;
}

void call_basic_sub(int crubase, char *subroutine) {
  enableROM(crubase);
  unsigned int addr = 0;
  unsigned int link = 0;

  struct DeviceRomHeader *dsrrom = (struct DeviceRomHeader *)0x4000;
  struct NameLink *entry = (struct NameLink *)dsrrom->basiclnk;
  while (entry != 0) {
    int elen = entry->name[0];
    char* entryname = entry->name + 1;
    char ebuf[8];
    bk_strncpy(ebuf, entryname, elen);

    if (0 == bk_strcmp(ebuf, subroutine)) {
      addr = entry->routine;
      link = (unsigned int)entry;
      break;
    }
    entry = entry->next;
  }
  disableROM(crubase);

  call_addr(crubase, addr, link);
}

void __attribute__((noinline)) call_lvl2(int crubase, unsigned char operation) {
  enableROM(crubase);
  unsigned int addr = 0;
  unsigned int link = 0;

  struct DeviceRomHeader* dsrrom = (struct DeviceRomHeader*) 0x4000;
  struct NameLink* entry = (struct NameLink*) dsrrom->basiclnk;
  unsigned int searchname = 0x0100 | operation;
  while(entry != 0) {
    unsigned int entryname = *((unsigned int*)entry->name);
    if (entryname == searchname) {
      addr = entry->routine;
      link = (unsigned int) entry;
      // Ensure the PAB name is in VDP
      unsigned int vdp = VPAB + 9;
      vdpmemcpy(vdp, entry->name, 2);
      PPAB = VPAB + 9 + 2;
      NCOMP = 1;
      break;
    }
    entry = entry->next;
  }
  disableROM(crubase);

  call_addr(crubase, addr, link);
}

static void call_addr(int crubase, int addr, int link) {
  if (addr == 0) {
    LVL2_STATUS = 0xFF;
    return;
  }

  // Setup scratchpad ram to look like traditional DSRLNK had occured
  // - console ROM dsrlnk leaves crubase at this address, too many people know about it.
  LVL2_CRULST = crubase;
  // - HRD ROS expects this to find ros executable to the subroutine name list.
  if (link != 0) {
    LVL2_SADDR = link;
  }

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
