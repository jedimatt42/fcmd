#ifndef _IOPORTS_H
#define _IOPORTS_H 1

//*********************
// VDP access ports
//*********************

// Read Data
#define VDPRD	*((volatile unsigned char*)0x8800)
// Read Status
#define VDPST	*((volatile unsigned char*)0x8802)
// Write Address/Register
#define VDPWA	*((volatile unsigned char*)0x8C02)
// Write Data
#define VDPWD	*((volatile unsigned char*)0x8C00)


// Set VDP address for read (no bit added)
inline void VDP_SET_ADDRESS(unsigned int x) { VDPWA = ((x) & 0xff); VDPWA = ((x) >> 8); }

// Set VDP address for write (adds 0x4000 bit)
inline void VDP_SET_ADDRESS_WRITE(unsigned int x) { VDPWA = ((x) & 0xff); VDPWA = (((x) >> 8) | 0x40); }

// Set VDP write-only register 'r' to value 'v'
inline void VDP_SET_REGISTER(unsigned char r, unsigned char v) { VDPWA = (v); VDPWA = (0x80 | (r)); }


#endif