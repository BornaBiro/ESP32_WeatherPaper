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
  uint8_t syncTimeout = 180;
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

uint8_t communication::saveDataToSD(struct sensorData *_s, struct data1StructHandle *_d1, struct data2StructHandle *_d2)
{
  SdFile _file;
  SdFat *_sd = _ink->getSdFat();
  char _tempStr[400];
  struct tm *_t;
  time_t _rtcEpoch;
  if (!_sd->begin(15, SD_SCK_MHZ(25))) return 0;
  _sd->chdir();

  _rtcEpoch = _rtc->getClock();
  getFolderPath(_tempStr, _rtcEpoch, COMMUNICATION_OUTDDOR);
  Serial.print("mkdir:");
  Serial.println(_tempStr);
  _sd->mkdir(_tempStr, true);
  _sd->chdir(_tempStr);

  // Create header on start of the file
  getFileNameFromEpoch(_tempStr, _rtcEpoch);
  Serial.print("file:");
  Serial.println(_tempStr);
  if (!_file.exists(_tempStr))
  {
    _file.open(_tempStr, O_RDWR | O_CREAT);
    _file.println(_outdoorDataHeader);
    _file.close();
  }
  if (_file.open(_tempStr, O_RDWR | O_AT_END))
  {
    _rtcEpoch = _rtc->getClock();
    _t = localtime((time_t*) & (_rtcEpoch));
    _file.timestamp(T_ACCESS | T_CREATE | T_WRITE, _t->tm_year + 1900, _t->tm_mon + 1, _t->tm_mday, _t->tm_hour, _t->tm_min, _t->tm_sec);
    _rtcEpoch = (time_t)(_d2->epoch);
    _t = localtime((const time_t*) & (_rtcEpoch));
    sprintf(_tempStr, _outdoorDataEntry, 0, _d2->epoch, _t->tm_mday, _t->tm_mon + 1, _t->tm_year + 1900, _t->tm_hour, _t->tm_min, _t->tm_sec, _d2->battery, _d1->tempSHT, _d1->tempSoil, _d1->humidity, _d1->pressure, _d1->light, _d1->uv / 100.0, _d2->solarJ, _d2->solarW, _d1->windSpeed, _d1->windDir, _d2->rain);
    _file.println(_tempStr);
    _file.close();
  }

  // Now store data from indoor unit
  _rtcEpoch = _rtc->getClock();
  getFolderPath(_tempStr, _rtcEpoch, COMMUNICATION_INDDOR);
  _sd->mkdir(_tempStr, true);
  _sd->chdir(_tempStr);

  getFileNameFromEpoch(_tempStr, _rtcEpoch);
  // Create header on start of the file
  if (!_file.exists(_tempStr))
  {
    _file.open(_tempStr, O_RDWR | O_CREAT);
    _file.println(_indoorDataHeader);
    _file.close();
  }
  if (_file.open(_tempStr, O_RDWR | O_AT_END))
  {
    _rtcEpoch = _rtc->getClock();
    _t = localtime((time_t*) & (_rtcEpoch));
    _file.timestamp(T_ACCESS | T_CREATE | T_WRITE, _t->tm_year + 1900, _t->tm_mon + 1, _t->tm_mday, _t->tm_hour, _t->tm_min, _t->tm_sec);
    _rtcEpoch = (time_t)(_s->timeStamp);
    _t = localtime((const time_t*) & (_rtcEpoch));
    sprintf(_tempStr, _indoorDataEntry, 0, _s->timeStamp, _t->tm_mday, _t->tm_mon + 1, _t->tm_year + 1900, _t->tm_hour, _t->tm_min, _t->tm_sec, _s->battery, _s->temp, _s->humidity, _s->pressure, _s->eco2, _s->tvoc, _s->rawH2, _s->rawEthanol);
    _file.println(_tempStr);
    _file.close();
  }
  return 1;
}

int16_t communication::getNumberOfEntries(time_t _epoch, uint8_t _indoor)
{
  int16_t _n = 0;
  SdFile _file;
  SdFat *_sd = _ink->getSdFat();
  struct tm *_t;
  time_t _rtcEpoch;
  char _temp[50];

  // Init SD card (just in case)
  if (!_sd->begin(15, SD_SCK_MHZ(25))) return -2;

  // Go to root
  _sd->chdir();

  _rtcEpoch = _rtc->getClock();
  _t = localtime((const time_t*)&_rtcEpoch);
  sprintf(_temp, _folderPath, _indoor == COMMUNICATION_INDDOR ? _folderIndoor : _folderOutdoor, _t->tm_year + 1900);
  if (_sd->chdir(_temp)) return -2;

  sprintf(_temp, _filenameStr, _t->tm_mday, _t->tm_mon + 1, _t->tm_year + 1900);
  if (!_file.open(_temp, O_RDONLY)) return -1;

  while (_file.available())
  {
    char c = _file.peek();
    if (c == '\n') _n;
    _file.seekCur(1);
  }
  _file.close();
  return _n;
}

uint8_t communication::getOutdoorDataFromSD(time_t _epoch, int16_t _n, struct data1StructHandle *_d1, struct data2StructHandle *_d2)
{
  char _temp[50];
  SdFile _file;
  SdFat *_sd = _ink->getSdFat();

  // Init SD card (just in case)
  if (!_sd->begin(15, SD_SCK_MHZ(25))) return 0;

  // Go to root
  _sd->chdir();

  // Go inside outdoor directory
  getFolderPath(_temp, _epoch, COMMUNICATION_OUTDDOR);
  if (!_sd->chdir(_temp)) return 0;

  // Open specific file
  getFileNameFromEpoch(_temp, _epoch);
  if (_file.open(_temp, O_RDONLY))
  {
    int _entries = 0;
    uint32_t *_dataStartPos = (uint32_t*)ps_malloc(sizeof(uint32_t) * (_n + 1));
    char *_oneLine = (char*)ps_malloc(sizeof(char) * (300));
    if (_dataStartPos == NULL || _oneLine == NULL) return 0;

    // Find start of each new line
    _file.rewind();
    while (_file.available() && _entries < (_n + 1))
    {
      if (_file.read() == '\n')
      {
        _dataStartPos[_entries] = _file.curPosition();
        Serial.println(_dataStartPos[_entries], DEC);
        _entries++;
      }
    }

    for (int i = 0; i < _n; i++)
    {
      // Copy each line in buffer
      uint32_t _size = _dataStartPos[i + 1] - _dataStartPos[i] - 2;
      _file.seekSet(_dataStartPos[i]);
      _file.read(_oneLine, _size);
      _oneLine[_size] = 0;
      _oneLine[_size + 1] = '\r';
      _oneLine[_size + 2] = '\n';
      float _uv;
      int _dummy1;
      int _dummy2;
      Serial.println(_oneLine);
      sscanf(_oneLine, _outdoorDataEntrySD, &_dummy1, &(_d2->epoch), &_dummy2, &_dummy2, &_dummy2, &_dummy2, &_dummy2, &_dummy2, &(_d2->battery), &(_d1->tempSHT), &(_d1->tempSoil), &(_d1->humidity), &( _d1->pressure), &(_d1->light), &_uv, &( _d2->solarJ), &(_d2->solarW), &(_d1->windSpeed), &(_d1->windDir), &(_d2->rain));
      _d1->uv = _uv * 100;
      Serial.println(_d2->battery, 2);
    }
    free(_dataStartPos);
    free(_oneLine);
    _file.close();
    return 1;
  }
  return 0;
}

void communication::getFileNameFromEpoch(char *_s, time_t _epoch)
{
  struct tm *_t;
  _t = localtime((const time_t*)&_epoch);
  sprintf(_s, _filenameStr, _t->tm_mday, _t->tm_mon + 1, _t->tm_year + 1900);
}

void communication::getFolderPath(char *_s, time_t _epoch, uint8_t _indoor)
{
  struct tm *_t;
  char _tempFileStr[40];
  _t = localtime((const time_t*)&_epoch);
  sprintf(_s, _folderPath, _indoor == COMMUNICATION_INDDOR ? _folderIndoor : _folderOutdoor, _t->tm_year + 1900);
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

  int minutes = (int)(ceil(((double) (_myTime.tm_min) + ((double) (_myTime.tm_sec) / 60)) / WAKEUP_INTERVAL) * WAKEUP_INTERVAL);
  _myTime.tm_min = minutes % 60;
  _myTime.tm_sec = 0;
  _alarmEpoch = mktime(&_myTime);

  if (minutes >= 60) _alarmEpoch += 3600;
  return _alarmEpoch;
}
