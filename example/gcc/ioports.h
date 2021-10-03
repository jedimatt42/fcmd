#ifndef _IOPORTS_H
#define _IOPORTS_H 1

#include "types.h"

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


// vdpmemset - sets a count of VDP memory bytes to a value
// Inputs: VDP address to start, the byte to set, and number of repeats
inline void vdp_memset(uint16_t pAddr, int ch, int cnt) {
    VDP_SET_ADDRESS_WRITE(pAddr);
    while (cnt--) {
        VDPWD = ch;
    }
}

// vdpmemcpy - copies a block of data from CPU to VDP memory
// Inputs: VDP address to write to, CPU address to copy from, number of bytes to copy
// void vdpmemcpy(int pAddr, const unsigned char *pSrc, int cnt);
//   inlining this will be about the same expense as a bankswitch call.
inline void vdp_memcpy(uint16_t pAddr, const char* pSrc, int cnt) {
    VDP_SET_ADDRESS_WRITE(pAddr);
    while (cnt--)
    {
        VDPWD = *(pSrc++);
    }
}

// vdpmemread - copies a block of data from VDP to CPU memory
// Inputs: VDP address to read from, CPU address to write to, number of bytes to copy
// void vdpmemread(int pAddr, unsigned char *pDest, int cnt);
inline void vdp_memread(uint16_t pAddr, char* pDest, int cnt) {
    VDP_SET_ADDRESS(pAddr);
    while (cnt--)
    {
        *(pDest++) = VDPRD;
    }
}

// vdpreadchar - read a character from VDP memory
// Inputs: VDP address to read
// Outputs: byte
// unsigned char vdpreadchar(int pAddr);
inline unsigned char vdp_readchar(uint16_t pAddr) {
    VDP_SET_ADDRESS(pAddr);
    __asm("NOP");
    return VDPRD;
}

#endif
