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

unsigned int loadDir(struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb) {
  struct PAB pab;

  struct VolInfo volInfo;
  struct DirEntry dirEntry;

  // CATALOG file must be INPUT | INTERNAL | RELATIVE
  // try with timestamps
  unsigned int ferr = bk_dsr_open(dsr, &pab, pathname, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_FIXED | DSR_TYPE_INTERNAL | DSR_TYPE_RELATIVE, 146);
  if (ferr) {
    // if that fails, try without
    unsigned int ferr = bk_dsr_open(dsr, &pab, pathname, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_FIXED | DSR_TYPE_INTERNAL | DSR_TYPE_RELATIVE, 38);
    if (ferr) {
      return ferr;
    }
  }

  int recNo = 0;
  ferr = DSR_ERR_NONE;
  while(ferr == DSR_ERR_NONE && request_break == 0) {
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
        volInfo.timestamps = (pab.CharCount > 38);
        vol_cb(&volInfo);
      } else {
        int namlen = bk_basicToCstr(cbuf, dirEntry.name);
        if (namlen == 0) {
          break;
        }
        char* cursor = cbuf+1+namlen;
        dirEntry.type = bk_ti_floatToInt(cursor);
        cursor += 9;
        dirEntry.sectors = bk_ti_floatToInt(cursor);
        cursor += 9;
        dirEntry.reclen = bk_ti_floatToInt(cursor);

        if (pab.CharCount > 38) {
          cursor += 9 /* point to 'creation time' */ +
            (6 * 9) /* and then to modified time */;
          dirEntry.ts_second = bk_ti_floatToInt(cursor);
          cursor += 9;
          dirEntry.ts_min = bk_ti_floatToInt(cursor);
          cursor += 9;
          dirEntry.ts_hour = bk_ti_floatToInt(cursor);
          cursor += 9;
          dirEntry.ts_day = bk_ti_floatToInt(cursor);
          cursor += 9;
          dirEntry.ts_month = bk_ti_floatToInt(cursor);
          cursor += 9;
          dirEntry.ts_year = bk_ti_floatToInt(cursor);
        } else {
          dirEntry.ts_year = 0;
          dirEntry.ts_month = 0;
          dirEntry.ts_day = 0;
          dirEntry.ts_hour = 0;
          dirEntry.ts_min = 0;
          dirEntry.ts_second = 0;
        }

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
