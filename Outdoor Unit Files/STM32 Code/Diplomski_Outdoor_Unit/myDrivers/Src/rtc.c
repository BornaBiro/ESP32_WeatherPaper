#include "rtc.h"

extern RTC_HandleTypeDef hrtc;

void RTC_SetTime(uint32_t _epoch)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    struct tm _myTime = RTC_EpochToHuman(_epoch);

    sTime.Hours = _myTime.tm_hour;
    sTime.Minutes = _myTime.tm_min;
    sTime.Seconds = _myTime.tm_sec;

    sDate.Date = _myTime.tm_mday;
    sDate.Month = _myTime.tm_mon;
    sDate.Year = (_myTime.tm_year) % 100;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

time_t RTC_GetTime()
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    struct tm _myTime;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    _myTime.tm_hour = sTime.Hours;
    _myTime.tm_min = sTime.Minutes;
    _myTime.tm_sec = sTime.Seconds;

    _myTime.tm_mday = sDate.Date;
    _myTime.tm_mon = sDate.Month;
    _myTime.tm_year = sDate.Year + 2000 - 1900;

	return mktime(&_myTime);
}


void RTC_SetAlarmEpoch(uint32_t _alarmEpoch, uint32_t _mask)
{
	HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
	RTC_AlarmTypeDef _sAlarm = {0};
	RTC_TimeTypeDef _sTime = {0};
	struct tm _myTime = RTC_EpochToHuman(_alarmEpoch);
	_sTime.Seconds = _myTime.tm_sec;
	_sTime.Minutes = _myTime.tm_min;
	_sTime.Hours = _myTime.tm_hour;
	_sAlarm.AlarmDateWeekDay = _myTime.tm_mday;
    _sAlarm.AlarmTime = _sTime;
    _sAlarm.AlarmMask = _mask;
    _sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    _sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    _sAlarm.Alarm = RTC_ALARM_A;
    HAL_RTC_SetAlarm_IT(&hrtc, &_sAlarm, RTC_FORMAT_BIN);
}

time_t RTC_HumanToEpoch(struct tm _t)
{
    return mktime(&_t);
}

struct tm RTC_EpochToHuman(time_t _epoch)
{
    struct tm _t;
    memcpy(&_t, localtime((const time_t*)(&_epoch)), sizeof(_t));
    return _t;
}
