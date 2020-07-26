#include "banks.h"
#define MYBANK BANK(9)

#include "b0_globals.h"
#include "b0_main.h"
#include "b2_dsrutil.h"
#include "b2_mds_dsrlnk.h"
#include "b2_tifloat.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include <vdp.h>
#include <string.h>

unsigned char loadDir(struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb) {
  struct PAB pab;

  struct VolInfo volInfo;
  struct DirEntry dirEntry;

  // specifying record length is not recommended (by TI)
  // CATALOG file must be INPUT | INTERNAL | RELATIVE
  unsigned int ferr = bk_dsr_open(dsr, &pab, pathname, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_FIXED | DSR_TYPE_INTERNAL | DSR_TYPE_RELATIVE, 0);
  if (ferr) {
    return ferr;
  }

  int recNo = 0;
  ferr = DSR_ERR_NONE;
  while(ferr == DSR_ERR_NONE) {
    unsigned char cbuf[150];
    ferr = bk_dsr_read(dsr, &pab, recNo);
    if (ferr == DSR_ERR_NONE) {
      // Now FBUF has the data...
      vdpmemread(FBUF, cbuf, pab.CharCount);
      // process Record
      if (recNo == 0) {
        int namlen = bk_basicToCstr(cbuf, volInfo.volname);
        if (namlen == 0) {
          tputs_rom("no device\n");
          break;
        }
        int a = bk_ti_floatToInt(cbuf+1+namlen);
        int j = bk_ti_floatToInt(cbuf+10+namlen);
        int k = bk_ti_floatToInt(cbuf+19+namlen);
        volInfo.total = j;
        volInfo.available = k;
        vol_cb(&volInfo);
      } else {
        int namlen = bk_basicToCstr(cbuf, dirEntry.name);
        if (namlen == 0) {
          break;
        }
        int a = bk_ti_floatToInt(cbuf+1+namlen);
        int j = bk_ti_floatToInt(cbuf+10+namlen);
        int k = bk_ti_floatToInt(cbuf+19+namlen);
        dirEntry.type = a;
        dirEntry.sectors = j;
        dirEntry.reclen = k;
        if (dirEntry.name[0] != 0) {
          dir_cb(&dirEntry);
        }
      }
      recNo++;
    } else {
      break;
    }
  }

  ferr = bk_dsr_close(dsr, &pab);
  if (ferr) {
    return ferr;
  }
}
