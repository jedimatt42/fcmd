#ifndef _DSR_H
#define _DSR_H 1

#include <files.h>

#define VPAB 0x2160
#define FBUF 0x2300

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
extern struct DeviceServiceRoutine* dsrList;

struct __attribute__((__packed__)) DirEntry {
  char name[11];
  int type;
  int sectors;
  int reclen;
  char ts_hour;
  char ts_min;
  char ts_second;
  int ts_year;
  char ts_month;
  char ts_day;
};

struct __attribute__((__packed__)) VolInfo {
  char volname[11];
  int total;
  int available;
  char timestamps;
  struct DeviceServiceRoutine* dsr;
};

unsigned int dsr_open(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen);
unsigned int dsr_close(struct DeviceServiceRoutine* dsr, struct PAB* pab);
unsigned int dsr_read(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber);
unsigned int dsr_write(struct DeviceServiceRoutine* dsr, struct PAB* pab, char* record, int reclen);
unsigned int dsr_status(struct DeviceServiceRoutine* dsr, struct PAB* pab);
unsigned int dsr_delete(struct DeviceServiceRoutine* dsr, struct PAB* pab);
unsigned int dsr_ea5load(struct DeviceServiceRoutine* dsr, const char* fname);
unsigned int dsr_reset(struct DeviceServiceRoutine* dsr, struct PAB* pab, int record);
unsigned int dsr_scratch(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber);

unsigned int dsr_prg_load(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int maxsize);
unsigned int dsr_prg_save(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int count);

typedef void (*vol_entry_cb)(struct VolInfo*);
typedef void (*dir_entry_cb)(struct DirEntry*);

unsigned int loadDir(struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb);
unsigned int existsDir(struct DeviceServiceRoutine* dsr, const char* pathname);
unsigned int existsFile(struct DeviceServiceRoutine* dsr, const char* pathname);

void loadDriveDSRs();

void enableROM(int crubase);
void disableROM(int crubase);

struct DeviceServiceRoutine* findDsr(char* devicename, int crubase);

void initPab(struct PAB* pab);

#include "banking.h"

DECLARE_BANKED_VOID(loadDriveDSRs, BANK(2), bk_loadDriveDSRs, (), ())
DECLARE_BANKED_VOID(initPab, BANK(2), bk_initPab, (struct PAB* pab), (pab))
DECLARE_BANKED_VOID(enableROM, BANK(2), bk_enableROM, (int crubase), (crubase))
DECLARE_BANKED_VOID(disableROM, BANK(2), bk_disableROM, (int crubase), (crubase))

DECLARE_BANKED(findDsr, BANK(2), struct DeviceServiceRoutine*, bk_findDsr, (char* devicename, int crubase), (devicename, crubase))
DECLARE_BANKED(existsDir, BANK(2), unsigned int, bk_existsDir, (struct DeviceServiceRoutine* dsr, const char* pathname), (dsr, pathname))
DECLARE_BANKED(existsFile, BANK(2), unsigned int, bk_existsFile, (struct DeviceServiceRoutine* dsr, const char* pathname), (dsr, pathname))
DECLARE_BANKED(dsr_delete, BANK(2), unsigned int, bk_dsr_delete, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))
DECLARE_BANKED(dsr_open, BANK(2), unsigned int, bk_dsr_open, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen),(dsr, pab, fname, flags, reclen))
DECLARE_BANKED(dsr_close, BANK(2), unsigned int, bk_dsr_close, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))
DECLARE_BANKED(dsr_read, BANK(2), unsigned int, bk_dsr_read, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber), (dsr, pab, recordNumber))
DECLARE_BANKED(dsr_write, BANK(2), unsigned int, bk_dsr_write, (struct DeviceServiceRoutine* dsr, struct PAB* pab, char* record, int reclen), (dsr, pab, record, reclen))
DECLARE_BANKED(dsr_reset, BANK(2), unsigned int, bk_dsr_reset, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int record), (dsr, pab, record))
DECLARE_BANKED(dsr_status, BANK(2), unsigned int, bk_dsr_status, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))
DECLARE_BANKED(dsr_scratch, BANK(2), unsigned int, bk_dsr_scratch, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int record), (dsr, pab, record))

DECLARE_BANKED(dsr_ea5load, BANK(9), unsigned int, bk_dsr_ea5load, (struct DeviceServiceRoutine * dsr, const char *fname), (dsr, fname))
DECLARE_BANKED(loadDir, BANK(9), unsigned int, bk_loadDir, (struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb), (dsr, pathname, vol_cb, dir_cb))

DECLARE_BANKED(dsr_prg_load, BANK(2), unsigned int, bk_dsr_prg_load, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int maxsize), (dsr, pab, fname, vdpaddr, maxsize))

DECLARE_BANKED(dsr_prg_save, BANK(2), unsigned int, bk_dsr_prg_save, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int count), (dsr, pab, fname, vdpaddr, count))

#endif
