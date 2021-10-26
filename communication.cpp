#include "communication.h"

communication::communication()
{

}

void communication::init(RF24_Inkplate * _rf, Inkplate *_dPtr, pcf85063 *_rtcPtr)
{
    _myRf = _rf;
    _d = _dPtr;
    _rtc = _rtcPtr;
}

void communication::setupCommunication()
{
  _myRf->setAutoAck(1);
  _myRf->enableAckPayload();
  _myRf->setChannel(0);
  _myRf->setDataRate(RF24_250KBPS);
  _myRf->setPALevel(RF24_PA_MAX, 1);
  _myRf->openWritingPipe(addr[1]);
  _myRf->openReadingPipe(1, addr[0]);
  _myRf->startListening();
}

uint8_t communication::sync(struct syncStructHandle *_s)
{
// Wait for sync
  uint8_t syncOk = 0;
  uint8_t syncTimeout = 120;
  _d->clearDisplay();
  _d->display();
  while (!syncOk && syncTimeout != 0)
  {
    _d->clearDisplay();
    _d->setCursor(0, 100);
    _d->print("Timeout: ");
    _d->print(syncTimeout--, DEC);
    _d->partialUpdate(false, true);
    if (_myRf->available())
    {
      char temp[32];
      while (_myRf->available())
      {
        _myRf->read(temp, 32);
      }
      if (temp[0] == SYNC_HEADER)
      {
        *_s = makeSyncStruct();
        _myRf->writeAckPayload(1, _s, sizeof(struct syncStructHandle));
        syncOk = 1;
      }
    }
    delay(1000);
  }
  _d->clearDisplay();
  
  _myRf->flush_tx();
  _myRf->flush_rx();
  _myRf->powerDown();
  return syncOk;
}

struct syncStructHandle communication::makeSyncStruct()
{
  syncStructHandle _s = {SYNC_HEADER};
  _s.myEpoch = _rtc->getClock();
  _s.sendEpoch = newWakeupTime(_s.myEpoch);
  return _s;
}

time_t communication::newWakeupTime(time_t _current)
{
  struct tm _myTime;
  time_t _alarmEpoch;
  memcpy (&_myTime, localtime((const time_t *) &_current), sizeof (_myTime));

  int minutes = (int)(ceil(((double) (_myTime.tm_min) + ((double) (_myTime.tm_sec) / 60)) / 2) * 2);
  _myTime.tm_min = minutes % 60;
  _myTime.tm_sec = 0;
  _alarmEpoch = mktime(&_myTime);

  if (minutes >= 60) _alarmEpoch += 3600;
  return _alarmEpoch;
}