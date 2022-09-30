#include "PCF85063.h"

pcf85063::pcf85063()
{

}

void pcf85063::readRegisters(uint8_t _reg, uint8_t *_data, uint8_t n)
{
  Wire.beginTransmission(PCF85063_ADDR);
  Wire.write(_reg);
  Wire.endTransmission(false);

  Wire.requestFrom((uint8_t)PCF85063_ADDR, n);

  while (Wire.available() != n);

  for (int i = 0; i < n; i++)
  {
    _data[i] = Wire.read();
  }
}

void pcf85063::writeRegisters(uint8_t _reg, uint8_t *_data, uint8_t n)
{
  Wire.beginTransmission(PCF85063_ADDR);
  Wire.write(_reg);
  Wire.write(_data, n);
  Wire.endTransmission();
}

void pcf85063::RTCinit(uint8_t resetRTC)
{
  uint8_t regData;
  if (resetRTC)
  {
    // Stop RTC
    regData = 0b00100000;
    writeRegisters(PCF85063_CTRL1, &regData, 1);
    delay(1);

    //Do a SW RST of RTC
    regData = 0b00110000;
    writeRegisters(PCF85063_CTRL1, &regData, 1);
    delay(1);
  }

  // Clear interrupts and set clockout frequency (1.024kHz)
  regData = PCF85063_CTRL2_MASK;
   writeRegisters(PCF85063_CTRL2, &regData, 1);

  // Start RTC normally with internal cap set to 12.5 pF
  regData = 0b00000000;
  writeRegisters(PCF85063_CTRL1, &regData, 1);
}

time_t pcf85063::getClock()
{
  uint8_t regData[7];
  struct tm _myTime;
  memset(&_myTime, 0, sizeof(_myTime));
  readRegisters(PCF85063_SECONDS, regData, 7);
  _myTime.tm_sec = bcd2dec(regData[0] & 0x7f);
  _myTime.tm_min = bcd2dec(regData[1] & 0x7f);
  _myTime.tm_hour = bcd2dec(regData[2] & 0x7f);
  _myTime.tm_mday = bcd2dec(regData[3] & 0x3f);
  _myTime.tm_wday = bcd2dec(regData[4] & 0x07);
  _myTime.tm_mon = bcd2dec(regData[5] & 0x1f);
  _myTime.tm_year = bcd2dec(regData[6]) + 2000 - 1900;
  _myTime.tm_isdst = 0;
  return mktime(&_myTime);
}

void pcf85063::setClock(time_t _epoch)
{
  struct tm _myTime;
  memset(&_myTime, 0, sizeof(_myTime));
  uint8_t regData[8];
  memcpy(&_myTime, localtime((const time_t*)&_epoch), sizeof(tm));
  // 0x0f sotred i RAM means that clock is alredy set
  regData[0] = 0x0f;
  regData[1] = dec2bcd(_myTime.tm_sec);
  regData[2] = dec2bcd(_myTime.tm_min);
  regData[3] = dec2bcd(_myTime.tm_hour);
  regData[4] = dec2bcd(_myTime.tm_mday);
  regData[5] = _myTime.tm_wday;
  regData[6] = dec2bcd(_myTime.tm_mon);
  regData[7] = dec2bcd((_myTime.tm_year + 1900) % 100);
  writeRegisters(PCF85063_RAMBYTE, regData, 8);
}

void pcf85063::setAlarm(time_t _epoch)
{
  uint8_t regData[5];
  struct tm _myTime;
  memset(&_myTime, 0, sizeof(_myTime));

  clearAlarm();
  disableAlarm();

  memcpy(&_myTime, localtime((const time_t*)&_epoch), sizeof(tm));
  regData[0] = dec2bcd(_myTime.tm_sec);
  regData[1] = dec2bcd(_myTime.tm_min);
  regData[2] = dec2bcd(_myTime.tm_hour);
  regData[3] = (1 << 7); // Disable alarm on day match.
  regData[4] = (1 << 7); // Disable alarm on weekday match.

  // Set up alarm time
  writeRegisters(PCF85063_SEC_AL, regData, 5);

  //Enable alarm on INT pin
  regData[0] = PCF85063_CTRL2_MASK | 0b10000000;
  writeRegisters(PCF85063_CTRL2, regData, 1);
}

void pcf85063::disableAlarm()
{
  //Disable alarm on INT pin
  uint8_t regData = PCF85063_CTRL2_MASK | 0b00000000;
  writeRegisters(PCF85063_CTRL2, &regData, 1);

  // And also clear the alarm flag
  clearAlarm();
}

bool pcf85063::checkAlarmFlag()
{
  uint8_t regData;
  readRegisters(PCF85063_CTRL2, &regData, 1);
  return regData & 0b01000000;
}

void pcf85063::clearAlarm()
{
  uint8_t regData = PCF85063_CTRL2_MASK;
  writeRegisters(PCF85063_CTRL2, &regData, 1);
}

uint8_t pcf85063::isClockSet()
{
  uint8_t regData;
  readRegisters(PCF85063_RAMBYTE, &regData, 1);
  return regData;
}