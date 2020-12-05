#include "banks.h"
#define MYBANK BANK(5)

#include "b5_clock.h"
#include "b0_globals.h"
#include "b2_dsrutil.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b4_variables.h"
#include <vdp.h>

void ide_clock(struct DateTime* dt);
void corcomp_clock(struct DateTime* dt, char* clock);

void datetime(struct DateTime* dt) {
  dt->day = 0;
  dt->dayOfWeek = 0;
  dt->hours = 0;
  dt->minutes = 0;
  dt->month = 0;
  dt->seconds = 0;
  dt->year = 0;
  dt->pm = 0;

  char* clock_type = bk_vars_get(str2ram("CLOCK"));
  if (clock_type == (char*) -1 || 0 == bk_strcmp(clock_type, str2ram("PI.CLOCK"))) {
    char clock[] = "PI.CLOCK";
    corcomp_clock(dt, clock);
  } else if (0 == bk_strcmp(clock_type, str2ram("CLOCK"))) {
    char clock[] = "CLOCK.";
    corcomp_clock(dt, clock);
  } else if (0 == bk_strcmp(clock_type, str2ram("IDE.TIME"))) {
    ide_clock(dt);
  }
}

void ide_clock(struct DateTime* dt) {
  // TODO, "IDE." needs to be added to legal device names...


  // 100 OPEN #1:"IDE.TIME", INTERNAL, FIXED
  // 110 INPUT #1:SEC$, MIN$, HOUR$, DAY$, MONTH$, YEAR$, DAYOFWEEK$
  // 120 CLOSE #1

  char clock[] = "IDE.TIME";
  struct DeviceServiceRoutine* dsr = 0;
  char namebuf[20];
  bk_parsePathParam(clock, &dsr, namebuf, PR_REQUIRED);
  if (dsr == currentDsr) {
    return;
  }
  struct PAB pab;

  int flags = DSR_TYPE_INPUT | DSR_TYPE_INTERNAL | DSR_TYPE_SEQUENTIAL | DSR_TYPE_FIXED;

  int err = bk_dsr_open(dsr, &pab, namebuf, flags, 0);
  if (err) {
    return;
  }

  char linebuf[22];
  err = bk_dsr_read(dsr, &pab, 0);
  if (err) {
    return;
  }
  vdpmemread(pab.VDPBuffer, linebuf, pab.CharCount);

  char numbuf[10];
  char* cursor = linebuf;
  cursor += 1 + bk_basicToCstr(cursor, numbuf);
  dt->seconds = bk_atoi(numbuf);
  cursor += 1 + bk_basicToCstr(cursor, numbuf);
  dt->minutes = bk_atoi(numbuf);
  cursor += 1 + bk_basicToCstr(cursor, numbuf);
  dt->hours = bk_atoi(numbuf);
  cursor += 1 + bk_basicToCstr(cursor, numbuf);
  dt->day = bk_atoi(numbuf);
  cursor += 1 + bk_basicToCstr(cursor, numbuf);
  dt->month = bk_atoi(numbuf);
  cursor += 1 + bk_basicToCstr(cursor, numbuf);
  dt->year = bk_atoi(numbuf);
  bk_basicToCstr(cursor, numbuf);
  dt->dayOfWeek = bk_atoi(numbuf) - 1;

  dt->pm = 0;
  if (dt->hours > 11) {
    dt->pm = 1;
    if (dt->hours > 12) {
      dt->hours -= 12;
    }
  }
  if (dt->hours == 0) {
    dt->hours = 12;
  }

  bk_dsr_close(dsr, &pab);
}

void corcomp_clock(struct DateTime* dt, char* clock) {
  struct DeviceServiceRoutine* dsr = 0;
  char namebuf[20];
  bk_parsePathParam(clock, &dsr, namebuf, PR_REQUIRED);
  if (dsr == currentDsr) {
    return;
  }

  struct PAB pab;

  int flags = DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_SEQUENTIAL | DSR_TYPE_VARIABLE;

  int err = bk_dsr_open(dsr, &pab, namebuf, flags, 0);
  if (err) {
    return;
  }

  char linebuf[22];
  err = bk_dsr_read(dsr, &pab, 0);
  if (err) {
    return;
  }

  vdpmemread(pab.VDPBuffer, linebuf, pab.CharCount);
  linebuf[pab.CharCount] = 0;

  // linebuf should contain something like: 5,20/12/31,23:59:00
  char* dow = bk_strtok(linebuf, ',');
  char* date = bk_strtok(0, ',');
  char* time = bk_strtok(0, ',');

  dt->dayOfWeek = bk_atoi(dow);
  dt->month = bk_atoi(bk_strtok(date, '/'));
  dt->day = bk_atoi(bk_strtok(0, '/'));
  dt->year = bk_atoi(bk_strtok(0, '/')) + 2000;
  dt->hours = bk_atoi(bk_strtok(time, ':'));
  dt->minutes = bk_atoi(bk_strtok(0, ':'));
  dt->seconds = bk_atoi(bk_strtok(0, ':'));

  dt->pm = 0;
  if (dt->hours > 11) {
    dt->pm = 1;
    if (dt->hours > 12) {
      dt->hours -= 12;
    }
  }
  if (dt->hours == 0) {
    dt->hours = 12;
  }

  bk_dsr_close(dsr, &pab);
}

void pretty_time(struct DateTime* dt) {
  bk_tputs_ram(bk_uint2str(dt->hours));
  bk_tputc(':');
  if (dt->minutes < 10) {
    bk_tputc('0');
  }
  bk_tputs_ram(bk_uint2str(dt->minutes));
  if (dt->pm) {
    bk_tputc('p');
  } else {
    bk_tputc('a');
  }
  bk_tputc('m');
}