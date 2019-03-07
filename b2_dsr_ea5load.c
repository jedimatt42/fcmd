#include "banks.h"
#define MYBANK BANK_2

#include "b0_globals.h"
#include "b0_main.h"
#include "b2_dsrutil.h"
#include "b2_mds_dsrlnk.h"
#include "b2_tifloat.h"
#include "b1cp_strutil.h"
#include <conio.h>
#include <string.h>


#define GPLWSR12	*((volatile unsigned int*)0x83F8)

void finish_load(int crubase, int VDPPAB, int lastcharaddr);

// Handle full of libti99 functions need to be replicated in ROM here
static void ea5_vdpmemread(int pAddr, unsigned char *pDest, int cnt) {
  VDP_SET_ADDRESS(pAddr);
  while (cnt--) {
    *(pDest++)=VDPRD;
  }
}

static unsigned char ea5_vdpreadchar(int pAddr) {
  VDP_SET_ADDRESS(pAddr);
  __asm("NOP");
  return VDPRD;
}

static void ea5_vdpchar(int pAddr, int ch) {
  VDP_SET_ADDRESS_WRITE(pAddr);
  VDPWD=ch;
}

unsigned int dsr_ea5load(struct DeviceServiceRoutine* dsr, const char* fname) {
  struct PAB* pab = (struct PAB*) 0x8320;
  initPab(pab);
  pab->OpCode = DSR_LOAD;
  pab->pName = (char*)fname;
  pab->NameLength = strlen(fname);
  pab->VDPBuffer = 0x1380;
  pab->RecordNumber = 0x2800;
  int VDPPAB = pab->VDPBuffer - 740;
  int crubase = dsr->crubase;
  unsigned char err = mds_dsrlnk(crubase, pab, VDPPAB, DSR_MODE_LVL3);
  if (err) {
    reboot();
  }
  int lastcharaddr = VDPPAB + pab->NameLength + 9;

  finish_load(crubase, VDPPAB, lastcharaddr);
}

#define FADDR (*(volatile int*)0x8320)
#define FLAG (*(volatile int*)0x8324)
#define BSIZE (*(volatile int*)0x8326)
#define ADDR (*(volatile int*)0x8328)

void finish_load(int crubase, int VDPPAB, int lastcharaddr) {
  FADDR = 0;
  FLAG = 0xFFFF;
  while(FLAG) {
    ea5_vdpmemread(0x1380, (char*)0x8324, 2);
    ea5_vdpmemread(0x1382, (char*)0x8326, 2);
    ea5_vdpmemread(0x1384, (char*)0x8328, 2);
    if (FADDR == 0) {
      FADDR = ADDR;
    }
    // after this point, expansion ram belongs to the target program
    ea5_vdpmemread(0x1386, (char*) ADDR, BSIZE);
    if (FLAG == 0) {
      __asm__(
        "bl *%0"
        : : "r" (FADDR)
      );
    } else {
      // increment name in vdp pab... 
      ea5_vdpchar(lastcharaddr, 1 + ea5_vdpreadchar(lastcharaddr));

      // now we can call it
      mds_dsrlnkraw(crubase, VDPPAB, DSR_MODE_LVL3);

      // if GPLWS(R12) is not crubase, then the dsr skipped the request
      if (!(GPLWSR12 == crubase && 0==GET_ERROR(ea5_vdpreadchar(VDPPAB+1)))) {
        reboot();
      }

    }
  }
}
