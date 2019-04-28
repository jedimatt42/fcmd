#ifndef _DSR_H
#define _DSR_H 1

#include <files.h>

#define VPAB 0x3200
#define FBUF 0x3000

// Casting rom locations to the next 3 structs should ease 
// reasoning about any code accessing the rom header and
// lists.

struct __attribute__((__packed__)) EntryLink {
  struct EntryLink* next;
  unsigned int* routine;
};

struct __attribute__((__packed__)) NameLink {
  struct NameLink* next;
  unsigned int routine;
  char name[8]; // length byte + [upto] 7 characters. 
};

struct __attribute__((__packed__)) DeviceRomHeader {
  unsigned char flag;
  unsigned char version;
  unsigned int reserved1;
  struct EntryLink* powerlnk;
  struct NameLink* userlnk;
  struct NameLink* dsrlnk;
  struct NameLink* basiclnk;
  struct EntryLink* interruptlnk;
  unsigned int* gromsomething;
};

// fun with 'drives'
struct __attribute__((__packed__)) DeviceServiceRoutine {
  char name[8];
  int crubase;
  unsigned int addr;
  char unit;
}; 

// A cache of dsr names and addresses.
extern struct DeviceServiceRoutine dsrList[40];

struct __attribute__((__packed__)) DirEntry {
  char name[11];
  int type;
  int sectors;
  int reclen;
};

struct __attribute__((__packed__)) VolInfo {
  char volname[11];
  int available;
  int total;
  struct DeviceServiceRoutine* dsr;
};

unsigned int dsr_open(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen);
unsigned int dsr_close(struct DeviceServiceRoutine* dsr, struct PAB* pab);
unsigned int dsr_read(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber);
unsigned int dsr_write(struct DeviceServiceRoutine* dsr, struct PAB* pab, unsigned char* record, int reclen);
unsigned int dsr_status(struct DeviceServiceRoutine* dsr, struct PAB* pab);
unsigned int dsr_delete(struct DeviceServiceRoutine* dsr, struct PAB* pab);
unsigned int dsr_ea5load(struct DeviceServiceRoutine* dsr, const char* fname);
unsigned int dsr_reset(struct DeviceServiceRoutine* dsr, struct PAB* pab, int record);

typedef void (*vol_entry_cb)(struct VolInfo*);
typedef void (*dir_entry_cb)(struct DirEntry*);

unsigned char loadDir(struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb);
unsigned int existsDir(struct DeviceServiceRoutine* dsr, const char* pathname);
unsigned int existsFile(struct DeviceServiceRoutine* dsr, const char* pathname);

void loadDriveDSRs();

void enableROM(int crubase);
void disableROM(int crubase);
int isDrive(char* basicstr);

struct DeviceServiceRoutine* findDsr(char* devicename, int crubase);

void initPab(struct PAB* pab);

#include "banking.h"

DECLARE_BANKED_VOID(loadDriveDSRs, BANK_2, bk_loadDriveDSRs, (), ())
DECLARE_BANKED_VOID(initPab, BANK_2, bk_initPab, (struct PAB* pab), (pab))

DECLARE_BANKED(findDsr, BANK_2, struct DeviceServiceRoutine*, bk_findDsr, (char* devicename, int crubase), (devicename, crubase))
DECLARE_BANKED(existsDir, BANK_2, unsigned int, bk_existsDir, (struct DeviceServiceRoutine* dsr, const char* pathname), (dsr, pathname))
DECLARE_BANKED(existsFile, BANK_2, unsigned int, bk_existsFile, (struct DeviceServiceRoutine* dsr, const char* pathname), (dsr, pathname))
DECLARE_BANKED(dsr_ea5load, BANK_2, unsigned int, bk_dsr_ea5load, (struct DeviceServiceRoutine* dsr, const char* fname), (dsr, fname))
DECLARE_BANKED(dsr_delete, BANK_2, unsigned int, bk_dsr_delete, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))
DECLARE_BANKED(dsr_open, BANK_2, unsigned int, bk_dsr_open, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen),(dsr, pab, fname, flags, reclen))
DECLARE_BANKED(dsr_close, BANK_2, unsigned int, bk_dsr_close, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))
DECLARE_BANKED(dsr_read, BANK_2, unsigned int, bk_dsr_read, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber), (dsr, pab, recordNumber))
DECLARE_BANKED(dsr_write, BANK_2, unsigned int, bk_dsr_write, (struct DeviceServiceRoutine* dsr, struct PAB* pab, unsigned char* record, int reclen), (dsr, pab, record, reclen))
DECLARE_BANKED(dsr_reset, BANK_2, unsigned int, bk_dsr_reset, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int record), (dsr, pab, record))
DECLARE_BANKED(dsr_status, BANK_2, unsigned int, bk_dsr_status, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))


#endif
