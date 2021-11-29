#ifndef __TIMEANDDATE_H__
#define __TIMEANDDATE_H__

#include "time.h"
#include "sys/time.h"

static const char DOW[7][4] = {"NED", "PON", "UTO", "SRI", "CET", "PET", "SUB"};

static struct tm epochToHuman(time_t _t)
{
  struct tm *_timePtr;
  struct tm _time;
  _timePtr = localtime(&_t);
  memcpy(&_time, _timePtr, sizeof(_time));
  return _time;
}

#endif