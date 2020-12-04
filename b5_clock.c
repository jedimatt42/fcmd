#include "banks.h"
#define MYBANK BANK(5)

#include "b5_clock.h"
#include "b2_dsrutil.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include <vdp.h>

void datetime(struct DateTime* dt) {
  struct DeviceServiceRoutine* dsr;
  char namebuf[20];
  bk_parsePathParam(str2ram("PI.CLOCK"), &dsr, namebuf, 0);

  struct PAB pab;

  int flags = DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_SEQUENTIAL | DSR_TYPE_VARIABLE;

  int err = bk_dsr_open(dsr, &pab, namebuf, flags, 0);
  if (!err) {
      char linebuf[256];
      err = bk_dsr_read(dsr, &pab, 0);
      if (!err) {
          vdpmemread(pab.VDPBuffer, linebuf, pab.CharCount);
          linebuf[pab.CharCount] = 0;

          // linebuf should contain something like: Wed,20/12/31,23:59:00
          char* tok = bk_strtok(linebuf, ',');
          if (tok[0] == 'S' && tok[1] == 'u') {
              dt->dayOfWeek = 0;
          } else if (tok[0] == 'M') {
              dt->dayOfWeek = 1;
          } else if (tok[0] == 'T' && tok[1] == 'u') {
              dt->dayOfWeek = 2;
          } else if (tok[0] == 'W') {
              dt->dayOfWeek = 3;
          } else if (tok[0] == 'T' && tok[1] == 'h') {
              dt->dayOfWeek = 4;
          } else if (tok[0] == 'F') {
              dt->dayOfWeek = 5;
          } else if (tok[1] == 'a') {
              dt->dayOfWeek = 6;
          }
          char* date = bk_strtok(0, ',');
          char* time = bk_strtok(0, ',');



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