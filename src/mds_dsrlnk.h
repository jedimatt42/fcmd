#ifndef mds_dsrlnk_h
#define mds_dsrlnk_h

#include "banking.h"

unsigned int mds_lvl3_dsrlnk(int crubase, struct PAB *pab, unsigned int vdp);
void mds_lvl3_dsrlnkraw(int crubase, unsigned int vdp);

DECLARE_BANKED(mds_lvl3_dsrlnk, BANK(2), unsigned int, bk_mds_lvl3_dsrlnk, (int crubase, struct PAB *pab, unsigned int vdp), (crubase, pab, vdp))
DECLARE_BANKED_VOID(mds_lvl3_dsrlnkraw, BANK(2), bk_mds_lvl3_dsrlnkraw, (int crubase, unsigned int vdp), (crubase, vdp))

#endif