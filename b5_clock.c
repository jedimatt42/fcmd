#include "banks.h"
#define MYBANK BANK(5)

#include "b5_clock.h"
#include "b2_dsrutil.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include <vdp.h>

void datetime(struct DateTime* dt) {
  struct DeviceServiceRoutine* dsr = 0;
  char namebuf[20];
  char device[] = "CLOCK.";
  bk_parsePathParam(device, &dsr, namebuf, PR_REQUIRED);
  if (dsr) {
    struct PAB pab;

    int flags = DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_SEQUENTIAL | DSR_TYPE_VARIABLE;

    int err = bk_dsr_open(dsr, &pab, namebuf, flags, 0);
    if (!err) {
      char linebuf[22];
      err = bk_dsr_read(dsr, &pab, 0);
      if (!err) {
        vdpmemread(pab.VDPBuffer, linebuf, pab.CharCount);
        linebuf[pab.CharCount] = 0;

        // linebuf should contain something like: Wed,20/12/31,23:59:00
        char* dow = bk_strtok(linebuf, ',');
        char* date = bk_strtok(0, ',');
        char* time = bk_strtok(0, ',');

        dt->dayOfWeek = bk_atoi(dow);
        dt->month = bk_atoi(bk_strtok(date, '/'));
        dt->day = bk_atoi(bk_strtok(0, '/'));
        dt->year = bk_atoi(bk_strtok(0, '/'));
        dt->hours = bk_atoi(bk_strtok(time, ':'));
        dt->minutes = bk_atoi(bk_strtok(0, ':'));
        dt->seconds = bk_atoi(bk_strtok(0, ':'));
      }
      bk_dsr_close(dsr, &pab);
      return;
    }
  }
  dt->day = 0;
  dt->dayOfWeek = 0;
  dt->hours = 0;
  dt->minutes = 0;
  dt->month = 0;
  dt->seconds = 0;
  dt->year = 0;
}