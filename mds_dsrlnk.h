#ifndef mds_dsrlnk_h
#define mds_dsrlnk_h

#define DSR_MODE_LVL2 10
#define DSR_MODE_LVL3 8

unsigned char mds_dsrlnk(int crubase, struct PAB *pab, unsigned int vdp, int mode);
void __attribute__((noinline)) mds_dsrlnkraw(int crubase, unsigned int vdp, int mode);

#endif