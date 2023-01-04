#ifndef __RTC_H__
#define __RTC_H__

#include "stdint.h"
#include "stm32l0xx_hal.h"
#include <time.h>
#include <string.h>

time_t RTC_GetTime();
void RTC_SetTime(uint32_t _epoch);
void RTC_SetAlarmEpoch(uint32_t _alarmEpoch, uint32_t _mask);
time_t RTC_HumanToEpoch(struct tm _t);
struct tm RTC_EpochToHuman(time_t _epoch);

#endif
