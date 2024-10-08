#include "banks.h"
#define MYBANK BANK(2)

#include "b0_globals.h"
#include "b2_dsrutil.h"
#include "b2_mds_dsrlnk.h"
#include "b1_strutil.h"
#include "b0_heap.h"
#include <vdp.h>
#include <string.h>

struct DeviceServiceRoutine* dsrList;

int matchesPrefix(char* basicstr, char* device_prefix) {
  return basicstr[1] == device_prefix[0] &&
    basicstr[2] == device_prefix[1] &&
    basicstr[3] == device_prefix[2];
}

static int __attribute__((noinline)) isDrive(char* basicstr) {

  // include IDE
  if (basicstr[0] == 3 && matchesPrefix(basicstr, "IDE")) {
    return 1;
  }

  // include CLOCK
  if (basicstr[0] == 5 && matchesPrefix(basicstr, "CLO")) {
    return 1;
  }

  // include PI
  if (basicstr[0] == 2 && basicstr[1] == 'P' && basicstr[2] == 'I') {
    return 1;
  }

  // include TIPI
  if (basicstr[0] == 4 && matchesPrefix(basicstr, "TIP")) {
    return 1;
  }

  // include MEXP-1 RD
  if (basicstr[0] == 2 && basicstr[1] == 'R' && basicstr[2] == 'D') {
    return 1;
  }

  // include 'DSK', experimental as unit 0 for jasonACT's pico cart
  if (basicstr[0] == 3 && matchesPrefix(basicstr, "DSK")) {
    return 1;
  }

  if (basicstr[0] == 4) {
    if (basicstr[4] >= '0' && basicstr[4] <= '9') {
      return matchesPrefix(basicstr, "DSK") ||
        matchesPrefix(basicstr, "IDE") ||
        matchesPrefix(basicstr, "SCS") ||
        matchesPrefix(basicstr, "WDS") ||
        matchesPrefix(basicstr, "HDX") ||
        matchesPrefix(basicstr, "URI");
    } else if (basicstr[4] >= 'A' && basicstr[4] <= 'Z') {
      return matchesPrefix(basicstr, "DSK");
    }
  }

  return 0;
}

unsigned int existsDir(struct DeviceServiceRoutine* dsr, const char* pathname) {
  struct PAB pab;
  initPab(&pab);
  unsigned int open_err = dsr_open(dsr, &pab, pathname, DSR_TYPE_INPUT | DSR_TYPE_FIXED | DSR_TYPE_INTERNAL | DSR_TYPE_RELATIVE, 0);
  unsigned int read_err = 0;
  if (!open_err) {
    read_err = dsr_read(dsr, &pab, 0);
    dsr_close(dsr, &pab);
  }
  return open_err || read_err;
}

unsigned int existsFile(struct DeviceServiceRoutine* dsr, const char* pathname) {
  struct PAB pab;
  initPab(&pab);
  pab.pName = (unsigned char*) pathname;
  return dsr_status(dsr, &pab) != 0x0080;
}

//---- the following are meant to be easy, not fast ----

void initPab(struct PAB* pab) {
  pab->OpCode = DSR_OPEN;
  pab->Status = DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL | DSR_TYPE_INPUT;
  pab->RecordLength = 0;
  pab->RecordNumber = 0;
  pab->ScreenOffset = 0;
  pab->NameLength = 0;
  pab->CharCount = 0;
  pab->VDPBuffer = FBUF;
}

unsigned int dsr_prg_load(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int maxsize) {
  initPab(pab);
  pab->OpCode = DSR_LOAD;
  pab->VDPBuffer = vdpaddr;
  pab->pName = (unsigned char*)fname;
  pab->RecordNumber = maxsize;

  return mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
}

unsigned int dsr_prg_save(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int count) {
  initPab(pab);
  pab->OpCode = DSR_SAVE;
  pab->VDPBuffer = vdpaddr;
  pab->pName = (unsigned char*)fname;
  pab->RecordNumber = count;

  return mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
}

unsigned int dsr_open(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen) {
  initPab(pab);
  pab->OpCode = DSR_OPEN;
  if (flags) {
    pab->Status = (unsigned char) flags;
  }
  if (reclen != 0) {
    pab->RecordLength = reclen;
  }
  pab->pName = (unsigned char*)fname;

  int res = mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
  vdpmemread(VPAB + 4, (char*) (&pab->RecordLength), 1);
  return res;
}

unsigned int dsr_close(struct DeviceServiceRoutine* dsr, struct PAB* pab) {
  pab->OpCode = DSR_CLOSE;
  return mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
}

unsigned int dsr_reset(struct DeviceServiceRoutine* dsr, struct PAB* pab, int record) {
  pab->OpCode = DSR_REWIND;
  pab->RecordNumber = record;
  return mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
}

unsigned int dsr_scratch(struct DeviceServiceRoutine* dsr, struct PAB* pab, int record) {
  pab->OpCode = DSR_SCRATCH;
  pab->RecordNumber = record;
  return mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
}

// the data read is in FBUF, the length read in pab->CharCount
// typically passing 0 in for record number will let the controller
// auto-increment it.
unsigned int dsr_read(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber) {
  return dsr_read_cpu(dsr, pab, recordNumber, 0);
}

unsigned int dsr_read_cpu(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber, char* recordBuf) {
  pab->OpCode = DSR_READ;
  pab->RecordNumber = recordNumber;
  pab->CharCount = 0;
  int useCpuBuffer = dsr->cpuSup && recordBuf != 0;
  if (useCpuBuffer) {
    pab->VDPBuffer = (int) recordBuf;
    pab->OpCode = pab->OpCode | 0x40;
  } else {
    pab->VDPBuffer = FBUF;
  }
  unsigned char result = mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
  vdpmemread(VPAB + 5, (char*) (&pab->CharCount), 1);
  if (! (pab->Status & DSR_TYPE_VARIABLE)) {
    pab->CharCount = pab->RecordLength;
  }

  if (recordBuf != 0 && !dsr->cpuSup)  {
    // If cpu buffers are not supported by dsr, then copy from VDP to record buffer ourselves
    vdpmemread(pab->VDPBuffer, recordBuf, pab->CharCount);
  }

  return result;
}

unsigned int dsr_write(struct DeviceServiceRoutine* dsr, struct PAB* pab, char* record, int reclen) {
  pab->OpCode = DSR_WRITE;
  pab->CharCount = reclen;
  if (dsr->cpuSup) {
    pab->VDPBuffer = (int) record;
    pab->OpCode = pab->OpCode | 0x40;
  } else {
    pab->VDPBuffer = FBUF; // in case we read with cpu buffer previously.
    vdpmemcpy(pab->VDPBuffer, record, reclen);
  }

  unsigned char result = mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
  return result;
}

unsigned int dsr_status(struct DeviceServiceRoutine* dsr, struct PAB* pab) {
  pab->OpCode = DSR_STATUS;

  unsigned int result = (int)mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
  if (result) {
    return result << 8;
  } else {
    return vdpreadchar(VPAB+8);
  }
}

unsigned int dsr_delete(struct DeviceServiceRoutine* dsr, struct PAB* pab) {
  pab->OpCode = DSR_DELETE;

  unsigned char result = mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
  return result;
}

static int isIDE(char* name) {
  return name[0] == 4 && name[1] == 'I' && name[2] == 'D' && name[3] == 'E';
}

static int isSCSI(char* name) {
  return name[0] == 4 && name[1] == 'S' && name[2] == 'C' && name[3] == 'S';
}

static int isHFDC(char* name) {
  return name[0] == 4 && name[1] == 'W' && name[2] == 'D' && name[3] == 'S';
}

static int inKnownCpuBufferDeviceNames(char* name) {
  // name will be a device name BASIC String.
  // we are looking for IDE, SCS, WDS... 
  return isSCSI(name) || isHFDC(name) || isIDE(name);
}

static int inKnownCpuBufferCallNames(char* name) {
  // Horizon RamDisk just uses DSK for device names, but has a CALL MENU routine.
  return (name[0] == 4 && name[1] == 'M' && name[2] == 'E' && name[3] == 'N' && name[4] == 'U');
}

static int supportsCpuBuffers(struct DeviceRomHeader* dsrrom) {
  // Examine attributes of the ROM header and determine if it a device known to
  // support CPU buffers in PABs and LVL2 IO
  if (dsrrom->flag != 0xAA) {
    return 0;
  }

  // TIPI DSR version bit 2 indicates cpu buffer
  if (dsrrom->version & 0x02 && dsrrom->basiclnk) { 
    char call_name[11];
    bk_basicToCstr(dsrrom->basiclnk->name, call_name);
    if (bk_strcmp(call_name, str2ram("TIPI")) == 0) {
      return 1;
    }
  } 

  // Other devices are just based on well known names in the DSR devices list
  struct NameLink* nameLink = dsrrom->dsrlnk;
  while(nameLink != 0) {
    if (inKnownCpuBufferDeviceNames(nameLink->name)) {
      return 1;
    }
    nameLink = nameLink->next;
  }
  // or in the BASIC call list
  nameLink = dsrrom->basiclnk;
  while(nameLink != 0) {
    if (inKnownCpuBufferCallNames(nameLink->name)) {
      return 1;
    }
    nameLink = nameLink->next;
  }
  return 0;
}

void loadDriveDSRs() {
  dsrList = (struct DeviceServiceRoutine*) bk_alloc(sizeof(struct DeviceServiceRoutine));
  struct DeviceServiceRoutine* listHead = dsrList;

  int cruscan = 0x1000;
  while(cruscan < 0x2000) {
    enableROM(cruscan);
    struct DeviceRomHeader* dsrrom = (struct DeviceRomHeader*) 0x4000;
    if (dsrrom->flag == 0xAA) {
      int cpuBufferSupport = supportsCpuBuffers(dsrrom);
      struct NameLink* dsrlinks = dsrrom->dsrlnk;
      while(dsrlinks != 0) {

        if (isDrive(dsrlinks->name)) {
          bk_basicToCstr(dsrlinks->name, listHead->name);
          listHead->crubase = cruscan;
          listHead->addr = dsrlinks->routine;
          listHead->cpuSup = cpuBufferSupport;
          listHead = (struct DeviceServiceRoutine*) bk_alloc(sizeof(struct DeviceServiceRoutine));
        }

        dsrlinks = dsrlinks->next;
      }
    }

    disableROM(cruscan);
    cruscan += 0x0100;
  }

  currentDsr = dsrList;
  bk_strcpy(currentPath, currentDsr->name);
  bk_strcat(currentPath, str2ram("."));
}

void enableROM(int crubase) {
  __asm__("mov %0,r12\n\tsbo 0" : : "r"(crubase) : "r12");
}

void disableROM(int crubase) {
  __asm__("mov %0,r12\n\tsbz 0" : : "r"(crubase) : "r12");
}

struct DeviceServiceRoutine* findDsr(char* devicename, int crubase) {
  int i = 0;

  while(dsrList[i].name[0] != 0) {
    if (crubase != 0) {
      if (dsrList[i].crubase == crubase && 0 == bk_strcmp(dsrList[i].name, devicename)) {
        return &dsrList[i];
      }
    } else {
      if (0 == bk_strcmp(dsrList[i].name, devicename)) {
        return &dsrList[i];
      }
    }
    i++;
  }

  return 0;
}
