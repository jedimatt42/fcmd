#include "banks.h"
#define MYBANK BANK(9)

#include "b0_globals.h"
#include "b0_main.h"
#include "b2_dsrutil.h"
#include "b2_mds_dsrlnk.h"
#include "b2_tifloat.h"
#include "b1_strutil.h"
#include <conio.h>
#include <string.h>

#define GPLWSR11  *((volatile unsigned int*)0x83F6)
#define GPLWSR12	*((volatile unsigned int*)0x83F8)

void finish_load(int crubase, int VDPPAB, int lastcharaddr, int namelen);

extern int* GOEA5;

inline static unsigned char ea5_vdpreadchar(int pAddr) {
  VDP_SET_ADDRESS(pAddr);
  __asm("NOP");
  return VDPRD;
}

inline static void ea5_vdpchar(int pAddr, int ch) {
  VDP_SET_ADDRESS_WRITE(pAddr);
  VDPWD=ch;
}

// This overlaps the temporary PAB
#define FADDR (*(volatile int *)0x8320)
#define FLAG (*(volatile int *)0x8324)
#define BSIZE (*(volatile int *)0x8326)
#define ADDR (*(volatile int *)0x8328)

unsigned int dsr_ea5load(struct DeviceServiceRoutine* dsr, const char* fname) {
  // Move the C stack into scratchpad
  __asm__(
    "li r10,>83C0"
  );
  struct PAB* pab = (struct PAB*) 0x8320;
  bk_initPab(pab);
  int namelen = bk_strlen(fname);
  pab->OpCode = DSR_LOAD;
  pab->pName = (char*)fname;
  pab->NameLength = namelen;
  pab->VDPBuffer = 0x1380;
  pab->RecordNumber = 0x2800;
  int VDPPAB = pab->VDPBuffer - 740;
  int crubase = dsr->crubase;

  vdpmemcpy(0, fname, pab->NameLength);

  unsigned int err = bk_mds_lvl3_dsrlnk(crubase, pab, VDPPAB);
  if (err) {
    reboot();
  }
  int lastcharaddr = VDPPAB + namelen + 9;
  namelen--; // adjust so it is now the screen offset when printing name.

  FADDR = 0;
  FLAG = 0xFFFF;
  while(FLAG) {
    vdpmemread(0x1380, (char*)0x8324, 2); // FLAG
    vdpmemread(0x1382, (char*)0x8326, 2); // BSIZE
    vdpmemread(0x1384, (char*)0x8328, 2); // ADDR
    if (FADDR == 0) {
      FADDR = ADDR;
    }
    // after this point, expansion ram belongs to the target program
    // TODO: eliminate bank stack.
    vdpmemread(0x1386, (char*) ADDR, BSIZE); // copy image from vdp to target cpu ram
    if (FLAG == 0) {
      GPLWSR11 = FADDR;
      // We aren't comming back from this, so feel free to lie to gcc.
      __asm__(
        "b @GOEA5"
      );
    } else {
      // increment name in vdp pab...
      unsigned char next_char = 1 + ea5_vdpreadchar(lastcharaddr);
      ea5_vdpchar(lastcharaddr, next_char);

      // update screen with new name character
      ea5_vdpchar(namelen, next_char);

      // now we can call it
      bk_mds_lvl3_dsrlnkraw(crubase, VDPPAB);

      // if GPLWS(R12) is not crubase, then the dsr skipped the request
      if (!(GPLWSR12 == crubase && 0==GET_ERROR(ea5_vdpreadchar(VDPPAB+1)))) {
        reboot();
      }

    }
  }
}
