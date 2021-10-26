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
        _myRf->writeAckPayload(1, _s, sizeof(syncStructHandle));
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

uint8_t communication::getData(struct syncStructHandle *_s, struct data1StructHandle *_d1, struct data2StructHandle *_d2)
{
  _myRf->powerUp();
  setupCommunication();

  uint8_t dataRx = 0;
  uint8_t rxTimeout = 25;
  while (dataRx != 3 && rxTimeout != 0)
  {
    if (_myRf->available())
    {
      char temp[32];
      while (_myRf->available())
      {
        _myRf->read(temp, 32);
      }
      if (temp[0] == DATA1_HEADER)
      {
        memcpy(_d1, temp, sizeof(data1StructHandle));
        *_s = makeSyncStruct();
        _myRf->writeAckPayload(1, _s, sizeof(syncStructHandle));
        dataRx |= 1;
      }
      if (temp[0] == DATA2_HEADER)
      {
        memcpy(_d2, temp, sizeof(data2StructHandle));
        *_s = makeSyncStruct();
        _myRf->writeAckPayload(1, _s, sizeof(syncStructHandle));
        dataRx |= 2;
      }
    }
    delay(1000);
    rxTimeout--;
  }
  _myRf->flush_tx();
  _myRf->flush_rx();
  _myRf->powerDown();
  return dataRx;
}

struct syncStructHandle communication::makeSyncStruct()
{
  syncStructHandle _s = {SYNC_HEADER};
  _s.myEpoch = (uint32_t)(_rtc->getClock());
  _s.sendEpoch = (uint32_t)newWakeupTime((time_t)_s.myEpoch);
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