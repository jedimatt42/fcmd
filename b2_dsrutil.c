#include "banks.h"
#define MYBANK BANK(2)

#include "b0_globals.h"
#include "b0_main.h"
#include "b2_dsrutil.h"
#include "b2_mds_dsrlnk.h"
#include "b2_tifloat.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b0_heap.h"
#include <vdp.h>
#include <string.h>

struct DeviceServiceRoutine* dsrList;

unsigned int existsDir(struct DeviceServiceRoutine* dsr, const char* pathname) {
  struct PAB pab;
  initPab(&pab);
  unsigned int open_err = dsr_open(dsr, &pab, pathname, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_FIXED | DSR_TYPE_INTERNAL | DSR_TYPE_RELATIVE, 0);
  if (open_err == 0) {
    dsr_close(dsr, &pab);
  }
  return open_err;
}

unsigned int existsFile(struct DeviceServiceRoutine* dsr, const char* pathname) {
  struct PAB pab;
  initPab(&pab);
  pab.pName = (char*) pathname;
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

unsigned int dsr_open(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen) {
  initPab(pab);
  pab->OpCode = DSR_OPEN;
  if (flags) {
    pab->Status = (unsigned char) flags;
  }
  if (reclen != 0) {
    pab->RecordLength = reclen;
  }
  pab->pName = (char*)fname;

  int res = mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
  vdpmemread(VPAB + 4, (&pab->RecordLength), 1);
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

// the data read is in FBUF, the length read in pab->CharCount
// typically passing 0 in for record number will let the controller
// auto-increment it.
unsigned int dsr_read(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber) {
  pab->OpCode = DSR_READ;
  pab->RecordNumber = recordNumber;
  pab->CharCount = 0;

  unsigned char result = mds_lvl3_dsrlnk(dsr->crubase, pab, VPAB);
  vdpmemread(VPAB + 5, (&pab->CharCount), 1);
  if (! (pab->Status & DSR_TYPE_VARIABLE)) {
    pab->CharCount = pab->RecordLength;
  }
  return result;
}

unsigned int dsr_write(struct DeviceServiceRoutine* dsr, struct PAB* pab, unsigned char* record, int reclen) {
  pab->OpCode = DSR_WRITE;
  pab->CharCount = reclen;
  vdpmemcpy(pab->VDPBuffer, record, reclen);

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

void loadDriveDSRs() {
  dsrList = (struct DeviceServiceRoutine*) bk_alloc(sizeof(struct DeviceServiceRoutine));
  struct DeviceServiceRoutine* listHead = dsrList;

  int cruscan = 0x1000;
  while(cruscan < 0x2000) {
    enableROM(cruscan);
    struct DeviceRomHeader* dsrrom = (struct DeviceRomHeader*) 0x4000;
    if (dsrrom->flag == 0xAA) {

      struct NameLink* dsrlinks = dsrrom->dsrlnk;

      while(dsrlinks != 0) {

        if (isDrive(dsrlinks->name)) {
          bk_basicToCstr(dsrlinks->name, listHead->name);
          listHead->crubase = cruscan;
          listHead->addr = dsrlinks->routine;
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

int matchesPrefix(char* basicstr, char* device_prefix) {
  return basicstr[1] == device_prefix[0] &&
    basicstr[2] == device_prefix[1] &&
    basicstr[3] == device_prefix[2];
}

int isDrive(char* basicstr) {
  if (basicstr[0] == 4) {
    char tipi[] = "TIPI";
    if (0 == bk_basic_strcmp(basicstr, tipi)) {
      return 1;
    } else if (basicstr[4] >= '0' && basicstr[4] <= '9') {
      return matchesPrefix(basicstr, "DSK") ||
             matchesPrefix(basicstr, "IDE") ||
             matchesPrefix(basicstr, "SCS") ||
             matchesPrefix(basicstr, "WDS") ||
             matchesPrefix(basicstr, "HDX") ||
             matchesPrefix(basicstr, "URI");
    } else if (basicstr[4] >= 'A' && basicstr[4] <= 'Z') {
      return matchesPrefix(basicstr, "DSK");
    }
  } else if (basicstr[0] == 2) {
    char pi[] = "PI";
    return 0 == bk_basic_strcmp(basicstr, pi);
  } else if (basicstr[0] == 5) {
    char clock[] = "CLOCK";
    return 0 == bk_basic_strcmp(basicstr, clock);
  }
  return 0;
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
