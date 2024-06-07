#include "banks.h"
#define MYBANK BANK(9)

#include "b0_globals.h"
#include "b0_main.h"
#include "b2_dsrutil.h"
#include "b2_mds_dsrlnk.h"
#include "b2_tifloat.h"
#include "b1_strutil.h"
#include "b0_sams.h"
#include "b9_resetSams.h"
#include <conio.h>
#include <string.h>

#define GPLWSR11  *((volatile unsigned int*)0x83F6)
#define GPLWSR12	*((volatile unsigned int*)0x83F8)

extern int* GOEA5;

inline static unsigned char ea5_vdpreadchar(int pAddr) {
  VDP_SET_ADDRESS(pAddr);
  __asm__("NOP");
  return VDPRD;
}

inline static void ea5_vdpchar(int pAddr, int ch) {
  VDP_SET_ADDRESS_WRITE(pAddr);
  VDPWD=ch;
}

// This overlaps the temporary PAB
#define FLAG (*(volatile int *)0x8324)
#define BSIZE (*(volatile int *)0x8326)
#define ADDR (*(volatile int *)0x8328)


__attribute__((noreturn, noinline)) static void ea5load(struct DeviceServiceRoutine* dsr, const char* fname) {
  struct PAB* pab = (struct PAB*) 0x8320;
  bk_initPab(pab);
  int namelen = bk_strlen(fname);
  pab->OpCode = DSR_LOAD;
  pab->pName = (unsigned char*)fname;
  pab->NameLength = namelen;
  pab->VDPBuffer = 0x1380;
  pab->RecordNumber = 0x2800;
  int VDPPAB = pab->VDPBuffer - 740;
  int crubase = dsr->crubase;

  vdpmemcpy(0, fname, namelen);

  // dsr and fname should not be accessed beyond here, resetting sams invalidates 
  // there addresses.
  bk_resetSams();

  unsigned int err = bk_mds_lvl3_dsrlnk(crubase, pab, VDPPAB);
  if (err) {
    reboot();
  }
  int lastcharaddr = VDPPAB + namelen + 9;
  namelen--; // adjust so it is now the screen offset when printing name.

  GPLWSR11 = 0;
  FLAG = 0xFFFF;

     // Move the C stack into scratchpad
  __asm__(
    "li r10, >83C0"
  );

  while(FLAG) {
    vdpmemread(0x1380, (char*) FLAG, 6); // FLAG, BSIZE, ADDR
    // For the first file, use the load address as the launch address
    if (GPLWSR11 == 0) {
      GPLWSR11 = ADDR;
    }
    // after this point, expansion ram belongs to the target program
    vdpmemread(0x1386, (char*) ADDR, BSIZE); // copy image from vdp to target cpu ram
    if (FLAG == 0) {
      // If the header flag is 0, then this is the last file to place, so launch it.
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

      // now load the next file into VPD
      bk_mds_lvl3_dsrlnkraw(crubase, VDPPAB);

      // if GPLWS(R12) is not crubase, then the dsr skipped the request
      // if ((GPLWSR12 != crubase || GET_ERROR(ea5_vdpreadchar(VDPPAB+1)))) {
      if (GET_ERROR(ea5_vdpreadchar(VDPPAB+1))) {
        // if there was any error, reboot the 4a
        reboot();
      }

    }
  }
  while(1) { }
}

void dsr_ea5load(struct DeviceServiceRoutine* dsr, const char* fname) {
  ea5load(dsr, fname);
}

