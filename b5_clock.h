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
  unsigned char pm;
};

void detectClock();
void datetime(struct DateTime* dt);
void clock_hook();
void drawBar();
void pretty_time(struct DateTime* dt);

DECLARE_BANKED_VOID(detectClock, BANK(5), bk_detectClock, (), ())
DECLARE_BANKED_VOID(datetime, BANK(5), bk_datetime, (struct DateTime* dt), (dt))
DECLARE_BANKED_VOID(clock_hook, BANK(5), bk_clock_hook, (), ())
DECLARE_BANKED_VOID(drawBar, BANK(5), bk_drawBar, (), ())

#endif