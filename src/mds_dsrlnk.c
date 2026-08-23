#include "banks.h"
#define MYBANK BANK(2)

// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include <files.h>
#include <vdp.h>
#include <string.h>
#include "mds_dsrlnk.h"
#include "detect_vdp.h"

#define GPLWSR12	*((volatile unsigned int*)0x83F8)

static void mds_vdp_set_address(unsigned int address, int write) {
	if (vdp_type == VDP_9938 || vdp_type == VDP_9958) {
		VDP_SET_REGISTER(0x0e, address >> 14);
	}
	if (write) VDP_SET_ADDRESS_WRITE(address & 0x3fff);
	else VDP_SET_ADDRESS(address & 0x3fff);
}

static void mds_vdp_memcpy(unsigned int address, const char* source, int count) {
	mds_vdp_set_address(address, 1);
	while (count--) VDPWD = *(source++);
}

static void mds_vdp_memread(unsigned int address, char* dest, int count) {
	mds_vdp_set_address(address, 0);
	while (count--) *(dest++) = VDPRD;
}


// NOTE: because this does not return the entire PAB back to you,
// if you need data from the DSR other than the error byte
// (ie: RECORD NUMBER), then you have to get it yourself!
// ---------- Matthew Splett -----------------
// This routine is modified handle error:0 as an error instead of
// cascading on to other devices.
unsigned int mds_lvl3_dsrlnk(int crubase, struct PAB *pab, unsigned int vdp) {
	unsigned char x;

	// copies your PAB to VDP and then executes the call through dsrlnkraw
	mds_vdp_memcpy(vdp, (const char*)pab, 9);
	// assumes vdpmemcpy leaves the VDP address in the right place!
	if (pab->NameLength == 0) {
		x = bk_strlen(pab->pName);
	} else {
		x= pab->NameLength;
	}
	VDPWD = x;

	// and the filename itself - note we assume 'x' is valid!
	unsigned char *p = pab->pName;
	while (x--) {
		VDPWD = *(p++);
	}

	// now we can call it
	mds_lvl3_dsrlnkraw(crubase, vdp);

	// if GPLWS(R12) is not crubase, then the dsr skipped the request
	if (GPLWSR12 != crubase) {
		return 0xff;
	}

	mds_vdp_memread(vdp, (char*)pab, 9);

	// now return the result
	mds_vdp_set_address(vdp + 1, 0);
	return GET_ERROR(VDPRD);
}
