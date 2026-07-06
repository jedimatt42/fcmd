#include "banks.h"
#define MYBANK BANK(2)

// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include <files.h>
#include <vdp.h>
#include <string.h>
#include "b2_mds_dsrlnk.h"

#define GPLWSR12	*((volatile unsigned int*)0x83F8)


// NOTE: because this does not return the entire PAB back to you,
// if you need data from the DSR other than the error byte
// (ie: RECORD NUMBER), then you have to get it yourself!
// ---------- Matthew Splett -----------------
// This routine is modified handle error:0 as an error instead of
// cascading on to other devices.
unsigned int mds_lvl3_dsrlnk(int crubase, struct PAB *pab, unsigned int vdp) {
	unsigned char x;

	// copies your PAB to VDP and then executes the call through dsrlnkraw
	vdpmemcpy(vdp, (const unsigned char*)pab, 9);
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

	vdpmemread(vdp, (unsigned char*)pab, 9);

	// now return the result
	return GET_ERROR(vdpreadchar(vdp+1));
}

