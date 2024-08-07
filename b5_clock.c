#include "banks.h"
#define MYBANK BANK(5)

#include "b5_clock.h"
#include "b0_globals.h"
#include "b2_dsrutil.h"
#include "b10_parsing.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b4_variables.h"
#include <vdp.h>

void ide_clock(struct DateTime* dt);
void corcomp_clock(struct DateTime* dt, char* clock);
void mbp_clock(struct DateTime* dt);
void update_ampm(struct DateTime* dt);

void detectClock() {
  char name[6];
  memcpy(name, "CLOCK", 6);
  char value[10];
  struct DeviceServiceRoutine* dsr;
  dsr = bk_findDsr(str2ram("CLOCK"), 0);
  if (dsr) {
    memcpy(value, "CLOCK", 6);
    bk_vars_set(name, value);
    return;
  }
  dsr = bk_findDsr(str2ram("PI"), 0);
  if (dsr) {
    memcpy(value, "PI.CLOCK", 9);
    bk_vars_set(name, value);
    return;
  }
  dsr = bk_findDsr(str2ram("IDE"), 0);
  if (dsr) {
    memcpy(value, "IDE.TIME", 9);
    bk_vars_set(name, value);
    return;
  }
  struct DateTime dt;
  mbp_clock(&dt);
  if (dt.month != 0 && dt.year == 0) {
    memcpy(value, "MBP", 4);
    bk_vars_set(name, value);
    return;
  }
}

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
  if (clock_type == (char*) -1 || 0 == bk_strcmpi(clock_type, str2ram("PI.CLOCK"))) {
    char clock[] = "PI.CLOCK";
    corcomp_clock(dt, clock);
  } else if (0 == bk_strcmpi(clock_type, str2ram("CLOCK"))) {
    char clock[] = "CLOCK.";
    corcomp_clock(dt, clock);
  } else if (0 == bk_strcmpi(clock_type, str2ram("IDE.TIME"))) {
    ide_clock(dt);
  } else if (0 == bk_strcmpi(clock_type, str2ram("MBP"))) {
    mbp_clock(dt);
  }
  update_ampm(dt);
}

void update_ampm(struct DateTime* dt) {
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
}

void ide_clock(struct DateTime* dt) {
  // 100 OPEN #1:"IDE.TIME", INTERNAL, FIXED
  // 110 INPUT #1:SEC$, MIN$, HOUR$, DAY$, MONTH$, YEAR$, DAYOFWEEK$
  // 120 CLOSE #1
  struct DeviceServiceRoutine* dsr = bk_findDsr(str2ram("IDE"), 0);

  if (dsr == 0) {
    return;
  }
  struct PAB pab;

  int flags = DSR_TYPE_INPUT | DSR_TYPE_INTERNAL | DSR_TYPE_SEQUENTIAL | DSR_TYPE_FIXED;

  int err = bk_dsr_open(dsr, &pab, str2ram("IDE.TIME"), flags, 0);
  if (err) {
    return;
  }

  char linebuf[30];
  err = bk_dsr_read_cpu(dsr, &pab, 0, linebuf);
  if (err) {
    return;
  }

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
  err = bk_dsr_read_cpu(dsr, &pab, 0, linebuf);
  if (err) {
    return;
  }

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

/* MBP & MBP II clock (no year) */

struct MBP_Time {
  unsigned char subsec_1_1000;
  unsigned char na1;
  unsigned char subsec_1_100;
  unsigned char na2;
  unsigned char seconds;
  unsigned char na3;
  unsigned char minutes;
  unsigned char na4;
  unsigned char hours;
  unsigned char na5;
  unsigned char day_of_week;
  unsigned char na6;
  unsigned char day;
  unsigned char na7;
  unsigned char month;
  unsigned char na8;
};

#define MBP_BCD ((volatile struct MBP_Time*)0x8640)

unsigned int bcd2dec(unsigned int bcd) {
  return bcd - (6 * (bcd / 16));
}

void mbp_clock(struct DateTime* dt) {
  dt->year = 0;
  dt->month = bcd2dec(MBP_BCD->month);
  dt->day = bcd2dec(MBP_BCD->day);
  dt->hours = bcd2dec(MBP_BCD->hours);
  dt->minutes = bcd2dec(MBP_BCD->minutes);
  dt->seconds = bcd2dec(MBP_BCD->seconds);
}
