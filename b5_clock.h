#ifndef _CLOCK_H
#define _CLOCK_H 1

#include "banking.h"

struct __attribute__((__packed__)) DateTime {
  unsigned char dayOfWeek;
  unsigned int year;
  unsigned char month;
  unsigned char day;
  unsigned char hours;
  unsigned char minutes;
  unsigned char seconds;
};

void datetime(struct DateTime* dt);

DECLARE_BANKED_VOID(datetime, BANK(5), bk_datetime, (struct DateTime* dt), (dt))

#endif