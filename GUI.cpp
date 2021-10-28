#include "GUI.h"

GUI::GUI()
{
    // Empty constructor
}

void GUI::init(Inkplate *_inkPtr)
{
  _ink = _inkPtr;
}

void GUI::drawMainScreen(struct sensorData *_sensor, struct currentWeatherHandle *_current, struct forecastListHandle *_forecastList, struct forecastDisplayHandle *_displayForecast, struct oneCallApiHandle *_one, struct measruementHandle *_d, struct tm *_time)
{
    char tmp[50];
    _ink->clearDisplay();
    _ink->setTextSize(1);
    _ink->fillRect(20, 360, 760, 3, BLACK);
    _ink->setFont(DISPLAY_FONT_SMALL);
    _ink->setTextSize(2);
    _ink->setCursor(1, 70);
    _ink->print(_current->weatherDesc);
    _ink->setCursor(1, 20);
    _ink->print(_current->city);
    _ink->setTextSize(1);
    _ink->drawBitmap(760, 10, refIcon, 30, 30, BLACK);
    _ink->drawBitmap(710, 5, iconAntenna, 40, 40, BLACK);
    _ink->drawBitmap(660, 5, iconNTPSync, 40, 40, BLACK);

    sprintf(tmp, "%1d:%02d %d/%d/%04d %3s", _time->tm_hour,  _time->tm_min,  _time->tm_mday,  _time->tm_mon + 1,  _time->tm_year + 1900, DOW[ _time->tm_wday]);

    _ink->setFont(DISPLAY_FONT);
    _ink->fillRect(4, 50, 792, 3, BLACK);
    _ink->setCursor(267, 35);
    _ink->print(tmp);

    _ink->drawBitmap(20, 85, weatherIcon(atoi(_current->weatherIcon)), 50, 50, BLACK);
    _ink->setTextSize(2);
    _ink->setCursor(80, 130);
    _ink->print(round(_current->temp), 0);
    _ink->setCursor(_ink->getCursorX() + 5, 110);
    _ink->print('o');
    _ink->setTextSize(1);
    _ink->drawBitmap(1, 140, iconTlak, 40, 40, BLACK);
    _ink->setCursor(45, 170);
    _ink->print(_current->pressureGnd, DEC);
    _ink->drawBitmap(145, 140, iconVlaga, 40, 40, BLACK);
    _ink->setCursor(195, 170);
    _ink->print(_current->humidity);
    _ink->drawBitmap(1, 190, iconVjetar, 40, 40, BLACK);
    _ink->setCursor(45, 220);
    _ink->print(_current->windSpeed, 1);
    _ink->print(" m/s | ");
    _ink->print(oznakeVjetar[int((_current->windDir / 22.5) + .5) % 16]);
    _ink->drawBitmap(1, 240, iconSunrise, 40, 40 , BLACK);
    sprintf(tmp, "%d:%02d", epochToHuman(_current->sunrise).tm_hour, epochToHuman(_current->sunrise).tm_min);
    _ink->setCursor(40, 270);
    _ink->print(tmp);
    _ink->drawBitmap(130, 240, iconSunset, 40, 40 , BLACK);
    sprintf(tmp, "%d:%02d", epochToHuman(_current->sunset).tm_hour, epochToHuman(_current->sunset).tm_min);
    _ink->setCursor(175, 270);
    _ink->print(tmp);

    _ink->drawBitmap(300, 70, indoorIcon, 40, 40, BLACK);
    _ink->drawBitmap(300, 140, iconTlak, 40, 40, BLACK);
    _ink->setCursor(350, 170);
    _ink->print(_sensor->pressure, 1);
    _ink->drawBitmap(300, 190, iconVlaga, 40, 40, BLACK);
    _ink->setCursor(350, 220);
    _ink->print(_sensor->humidity, 1);
    _ink->drawBitmap(300, 240, iconTemp, 40, 40, BLACK);
    _ink->setCursor(350, 270);
    _ink->print(_sensor->temp, 1);

    if (strlen(_one->alertEvent)) _ink->drawBitmap(610, 5, iconWarning, 40, 40, BLACK);

    int xOffset, xOffsetText;
    for (int i = 1; i < 6; i++) 
    {
        xOffset = i * 160;
        xOffsetText = ((i - 1) * 160) + 10;
        _ink->fillRect(xOffset, 410, 3, 180, BLACK);
        _ink->drawBitmap(xOffset - 105, 420, weatherIcon(atoi(_displayForecast[i - _forecastList->shiftDay].weatherIcon)), 50, 50, BLACK);
        _ink->setTextSize(1);
        _ink->setFont(DISPLAY_FONT_SMALL);
        printAlignText(_displayForecast[i - _forecastList->shiftDay].weatherDesc, xOffset - 80, 482, ALIGMENT_CENTERTOP);
        sprintf(tmp, "%d.%d", epochToHuman(_forecastList->forecast[_forecastList->startElement[i - _forecastList->shiftDay]].timestamp).tm_mday, epochToHuman(_forecastList->forecast[_forecastList->startElement[i -_forecastList->shiftDay]].timestamp).tm_mon + 1);
        printAlignText(tmp, xOffset - 80, 415, ALIGMENT_CENTERTOP);
        sprintf(tmp, "%d hPa   %d %%", _displayForecast[i - _forecastList->shiftDay].avgPressure, _displayForecast[i - _forecastList->shiftDay].avgHumidity);
        printAlignText(tmp, xOffset - 80, 575, ALIGMENT_CENTERTOP);
        sprintf(tmp, "%.1f / %.1f m/s   %s", _displayForecast[i - _forecastList->shiftDay].avgWindSpeed, _displayForecast[i - _forecastList->shiftDay].maxWindSpeed, oznakeVjetar[int((_displayForecast[i - _forecastList->shiftDay].avgWindDir / 22.5) + .5) % 16]);
        printAlignText(tmp, xOffset - 80, 588, ALIGMENT_CENTERTOP);
        _ink->setFont(DISPLAY_FONT);
        sprintf(tmp, "%d | %d", _displayForecast[i - _forecastList->shiftDay].maxTemp, _displayForecast[i - _forecastList->shiftDay].minTemp);
        printAlignText(tmp, xOffset - 80, 520, ALIGMENT_CENTERTOP);
        printAlignText((char*)DOW[epochToHuman(_forecastList->forecast[_forecastList->startElement[i - _forecastList->shiftDay]].timestamp).tm_wday], xOffset - 80, 400, ALIGMENT_CENTERTOP);
    }

    for (int i = 1; i < 3; i++)
    {
        _ink->fillRect(267 * i, 60, 3, 290, BLACK);
    }

    _ink->setFont(DISPLAY_FONT);
    _ink->setCursor(600, 150);
    _ink->print(_d->tempSHT, 1);
    _ink->setCursor(600, 250);
    _ink->print(_d->humidity, 1);
    _ink->display();
}

void GUI::printAlignText(char *text, int16_t x, int16_t y, enum alignment align)
{
  int16_t x1, y1;
  uint16_t w, h;
  _ink->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  switch (align)
  {
    case ALIGMENT_CENTERBOT:
    case ALIGMENT_CENTER:
    case ALIGMENT_CENTERTOP:
      x1 = x - w / 2;
      break;
    case ALIGMENT_RIGHTBOT:
    case ALIGMENT_RIGHT:
    case ALIGMENT_RIGHTTOP:
      x1 = x;
      break;
    case ALIGMENT_LEFTBOT:
    case ALIGMENT_LEFT:
    case ALIGMENT_LEFTTOP:
    default:
      x1 = x - w;
      break;
  }
  switch (align)
  {
    case ALIGMENT_CENTERBOT:
    case ALIGMENT_RIGHTBOT:
    case ALIGMENT_LEFTBOT:
      y1 = y - h;
      break;
    case ALIGMENT_CENTER:
    case ALIGMENT_RIGHT:
    case ALIGMENT_LEFT:
      y1 = y - (h / 2);
      break;
    case ALIGMENT_CENTERTOP:
    case ALIGMENT_LEFTTOP:
    case ALIGMENT_RIGHTTOP:
    default:
      y1 = y;
      break;
  }
  _ink->setCursor(x1, y1);
  _ink->print(text);
}

void GUI::drawGraph(int16_t _x, int16_t _y, uint16_t _w, uint16_t _h, void *_xData, void *_yData, uint8_t _n, uint8_t _step, uint8_t _m, enum graphDataType _dataType, graphStyle _style, float _min, float _max)
{
  // First copy data into local array
  double *_intArray;
  _intArray = (double*)malloc(_n * sizeof(double));
  if (_intArray == NULL) return;

  // Fill the array with the data
  for (int i = 0; i < _n; i++)
  {
    switch (_dataType)
    {
      case DATATYPE_FLOAT:
        _intArray[i] = (*(float*)((uint8_t*)(_yData) + (_step * i)));
        break;
      case DATATYPE_DOUBLE:
        _intArray[i] = (*(double*)((uint8_t*)(_yData) + (_step * i)));
        break;
      case DATATYPE_UINT8_T:
        _intArray[i] = (*((uint8_t*)(_yData) + (_step * i)));
        break;
      case DATATYPE_UINT16_T:
        _intArray[i] = (*(uint16_t*)((uint8_t*)(_yData) + (_step * i)));
        break;
      case DATATYPE_INT16_T:
        _intArray[i] = (*(int16_t*)((uint8_t*)(_yData) + (_step * i)));
        break;
      case DATATYPE_INT:
        _intArray[i] = (*(int*)((uint8_t*)(_yData) + (_step * i)));
        break;
    }
  }

  // Now draw the y and x axis
  _ink->fillRect(_x, _y, 2, _h, BLACK);
  _ink->fillRect(_x, _y + _h, _w, 2, BLACK);
  struct tm *_time;

  // Draw time data on x axis
  uint8_t _xSpacing = _w / _n;
  uint8_t _ySpacing = _h / _m;
  for (int i = 0; i < _n; i++)
  {
    char temp[10];
    time_t _epoch = (*(uint32_t*)((uint8_t*)(_xData) + (_step * i)));
    _time = localtime((const time_t*)&_epoch);
    _ink->drawFastVLine(_x + (i * _xSpacing) + (_xSpacing / 2), _y, _h + 3, BLACK);
    sprintf(temp, "%d:%02d", _time->tm_hour, _time->tm_min);
    printAlignText(temp, _x + (i * _xSpacing) + (_xSpacing / 2), _y + _h + 10, ALIGMENT_CENTERTOP);
  }

  // Find max and min value
  float _dataMax = _intArray[0];
  float _dataMin = _dataMax;
  for (int i = 1; i < _n; i++)
  {
    if (isnan(_max)) if (_intArray[i] > _dataMax) _dataMax = _intArray[i];
    if (isnan(_min)) if (_intArray[i] < _dataMin) _dataMin = _intArray[i];
  }
  if (_dataMin == _dataMax) _dataMax = _dataMin + 1;
  if (!isnan(_max)) _dataMax = _max;
  if (!isnan(_min)) _dataMin = _min;

  // Now find data step
  float _yStep = (_dataMax - _dataMin) / _m;

  // Draw data on y axis
  for (int i = 0; i < (_m + 1); i++)
  {
    char temp[20];
    sprintf(temp, "%.1f", _dataMax - (i * _yStep));
    //_d->setCursor(_x - 30, _y + (_ySpacing * i) - 3);
    _ink->drawFastHLine(_x - 2, _y + (_ySpacing * i), _w, BLACK);
    //_d->print(temp);
    printAlignText(temp, _x - 5, _y + (_ySpacing * i), ALIGMENT_LEFT);

  }

  // Now draw all the data on graph
  switch (_style)
  {
    case GRAPHSTYLE_LINE:
      for (int i = 0; i < (_n - 1); i++)
      {
        _ink->drawThickLine(_x + (i * _xSpacing) + (_xSpacing / 2), (int16_t)map2(_intArray[i], _dataMin, _dataMax, _y + _h, _y), _x + ((i + 1) * _xSpacing) + (_xSpacing / 2), (int16_t)map2(_intArray[i + 1], _dataMin, _dataMax, _y + _h, _y), BLACK, 2);
      }
      break;
    case GRAPHSTYLE_COLUMN:
      for (int i = 0; i < _n; i++)
      {
        int16_t _columnY = (int16_t)map2(_intArray[i], _dataMin, _dataMax, _y + _h, _y);
        int16_t _columnH = _y + _h - _columnY;
        _ink->fillRect(_x + (i * _xSpacing) + 4, _columnY, _xSpacing - 8, _columnH, BLACK);
      }
      break;
    case GRAPHSTYLE_DOT:
      for (int i = 0; i < _n; i++)
      {
        _ink->fillCircle(_x + (i * _xSpacing) + (_xSpacing / 2) , (int16_t)map2(_intArray[i], _dataMin, _dataMax, _y + _h, _y), 5, BLACK);
      }
      break;
  }
  free(_intArray);
}

void GUI::drawOutdoorData(communication *_comm, time_t _epoch, uint32_t _dayOffset, uint16_t *_dataOffset, uint8_t _graph)
{
  struct measruementHandle *sdData;
  time_t _newTime = _epoch - (86400 * _dayOffset);
  char _tempStr[60];
  uint8_t _showNData = 8;

   _ink->clearDisplay();
  // Draw arrows for selecting the day
  _ink->fillTriangle(50, 10, 50, 40, 20, 25, BLACK);
  if (_dayOffset != 0) _ink->fillTriangle(750, 10, 750, 40, 780, 25, BLACK);
  // Draw back arrow
  _ink->fillTriangle(10, 580, 30, 570, 30, 590, BLACK);
  // Draw top line
  _ink->fillRect(4, 50, 792, 3, BLACK);
  // Draw text
  _ink->setFont(DISPLAY_FONT);
  _ink->setTextSize(1);
  _ink->setCursor(200, 35);
  sprintf(_tempStr, "Vanjska jed. %d.%02d.%04d, %s", epochToHuman(_newTime).tm_mday, epochToHuman(_newTime).tm_mon, epochToHuman(_newTime).tm_year + 1900, DOW[epochToHuman(_newTime).tm_wday]);
  _ink->setCursor(200, 35);
  _ink->print(_tempStr);
  // Draw bottom line and icons
  const uint8_t *iconList[] = {iconTemp, iconTlak, iconVlaga, iconVjetar, iconWindDir, iconLight, iconUV, iconSolar, iconSolar, iconRainDrop, iconBattery};
  for (int i = 0; i < 11; i++)
  {
    _ink->drawBitmap((i * 70) + 50, 550, iconList[i], 40, 40, BLACK);
  }
  _ink->setFont();
  _ink->setTextSize(2);
  int16_t _enteries = _comm->getNumberOfEntries(_newTime, COMMUNICATION_OUTDDOR);
  if (_enteries == -1 || _enteries < 1)
  {
    printAlignText("Nema podataka", 400, 300, ALIGMENT_CENTER);
  }
  else if (_enteries > 1)
  {
    // If data has been shifted so far, get it back!
    //if (((*_dataOffset) + 8) > _enteries) (*_dataOffset) = _enteries - 8;
    // Read all the data from SD
    sdData = (struct measruementHandle *)ps_malloc(sizeof(measruementHandle) * _enteries);
    if (sdData != NULL && _enteries > 0)
    {
      if (!_comm->getOutdoorDataFromSD(_newTime, _enteries, sdData))
      {
        _ink->setFont(DISPLAY_FONT);
        _ink->setTextSize(1);
        printAlignText("Pogreska s SD karticom", 400, 300, ALIGMENT_CENTER);
      }
      if (_enteries <= 8)
      {
        _showNData = _enteries;
        *_dataOffset = 0;
      }
      else
      {
        if ((*_dataOffset) + 8 > _enteries) (*_dataOffset) = _enteries - 8;
      }
      // Draw arrows for selecting visible data span
      if ((*_dataOffset) != 0) _ink->fillTriangle(120, 140, 120, 170, 90, 155, BLACK);
      if (((*_dataOffset) + 8) < _enteries) _ink->fillTriangle(680, 140, 680, 170, 710, 155, BLACK);
      switch (_graph)
      {
        case 0:
          printAlignText("Temperatura zraka [C]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].tempSHT), _showNData, sizeof(struct measruementHandle), 10, DATATYPE_FLOAT, GRAPHSTYLE_LINE);
          break;
        case 1:
          printAlignText("Tlak zraka [hPa]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].pressure), _showNData, sizeof(struct measruementHandle), 10, DATATYPE_FLOAT, GRAPHSTYLE_LINE);
          break;
        case 2:
          printAlignText("Relat. vlaznost zraka [%]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].humidity), _showNData, sizeof(struct measruementHandle), 10, DATATYPE_FLOAT, GRAPHSTYLE_LINE, 0, 100);
          break;
        case 3:
          printAlignText("Brzina vjetra [m/s]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].windSpeed), _showNData, sizeof(struct measruementHandle), 10, DATATYPE_FLOAT, GRAPHSTYLE_LINE);
          break;
        case 4:
          printAlignText("Smjer vjetra [stupnjevi]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].windDir), _showNData, sizeof(struct measruementHandle), 10, DATATYPE_INT16_T, GRAPHSTYLE_DOT, 0, 360);
          break;
        case 5:
          printAlignText("Kolicina svjetlosti [lux]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].light), _showNData, sizeof(struct measruementHandle), 10, DATATYPE_FLOAT, GRAPHSTYLE_LINE);
          break;
        case 6:
          printAlignText("UV zracenje [UV index]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].uv), _showNData, sizeof(struct measruementHandle), 10, DATATYPE_FLOAT, GRAPHSTYLE_COLUMN, 0, 12);
          break;
        case 7:
          printAlignText("Kolicina suncevog zracenja [J/cm2]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].solarJ), _showNData, sizeof(struct measruementHandle), 10, DATATYPE_FLOAT, GRAPHSTYLE_COLUMN);
          break;
        case 8:
          printAlignText("Kolicina suncevog zracenja [W/m2]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].solarW), _showNData, sizeof(struct measruementHandle), 10, DATATYPE_FLOAT, GRAPHSTYLE_COLUMN);
          break;
        case 9:
          printAlignText("Kolicina padalina [mm]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].rain), _showNData, sizeof(struct measruementHandle), 10, DATATYPE_FLOAT, GRAPHSTYLE_COLUMN);
          break;
        case 10:
          printAlignText("Baterija [V]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].battery), _showNData, sizeof(struct measruementHandle), 10, DATATYPE_FLOAT, GRAPHSTYLE_COLUMN, 0, 4.4);
          break;
      }
      free(sdData);
    }
  }
}

void GUI::drawIndoorData(communication *_comm, time_t _epoch, uint32_t _dayOffset, uint16_t *_dataOffset, uint8_t _graph)
{
  struct sensorData *sdData;
  time_t _newTime = _epoch - (86400 * _dayOffset);
  char _tempStr[60];
  uint8_t _showNData = 8;

   _ink->clearDisplay();
  // Draw arrows for selecting the day
  _ink->fillTriangle(50, 10, 50, 40, 20, 25, BLACK);
  if (_dayOffset != 0) _ink->fillTriangle(750, 10, 750, 40, 780, 25, BLACK);
  // Draw back arrow
  _ink->fillTriangle(10, 580, 30, 570, 30, 590, BLACK);
  // Draw top line
  _ink->fillRect(4, 50, 792, 3, BLACK);
  // Draw text
  _ink->setFont(DISPLAY_FONT);
  _ink->setTextSize(1);
  _ink->setCursor(200, 35);
  sprintf(_tempStr, "Unutrasnja jed. %d.%02d.%04d, %s", epochToHuman(_newTime).tm_mday, epochToHuman(_newTime).tm_mon, epochToHuman(_newTime).tm_year + 1900, DOW[epochToHuman(_newTime).tm_wday]);
  _ink->setCursor(200, 35);
  _ink->print(_tempStr);
  // Draw bottom line and icons
  const uint8_t *iconList[] = {iconTemp, iconTlak, iconVlaga, iconCo2, iconTvoc, iconH2, iconEth, iconBattery};
  for (int i = 0; i < 8; i++)
  {
    _ink->drawBitmap((i * 70) + 50, 550, iconList[i], 40, 40, BLACK);
  }

  _ink->setFont();
  _ink->setTextSize(2);
  int16_t _enteries = _comm->getNumberOfEntries(_newTime, COMMUNICATION_INDDOR);
  if (_enteries == -1 || _enteries < 1)
  {
    printAlignText("Nema podataka", 400, 300, ALIGMENT_CENTER);
  }
  else if (_enteries > 1)
  {
    // If data has been shifted so far, get it back!
    if (((*_dataOffset) + 8) > _enteries) (*_dataOffset) = _enteries - 8;
    // Read all the data from SD
    sdData = (struct sensorData *)ps_malloc(sizeof(sensorData) * _enteries);
    if (sdData != NULL && _enteries > 0)
    {
      if (!_comm->getIndoorDataFromSD(_newTime, _enteries, sdData))
      {
        _ink->setFont(DISPLAY_FONT);
        _ink->setTextSize(1);
        printAlignText("Pogreska s SD karticom", 400, 300, ALIGMENT_CENTER);
      }
      if (_enteries <= 8)
      {
        _showNData = _enteries;
        *_dataOffset = 0;
      }
      else
      {
        if ((*_dataOffset) + 8 > _enteries) (*_dataOffset) = _enteries - 8;
      }
      // Draw arrows for selecting visible data span
      if ((*_dataOffset) != 0) _ink->fillTriangle(120, 140, 120, 170, 90, 155, BLACK);
      if (((*_dataOffset) + 8) < _enteries) _ink->fillTriangle(680, 140, 680, 170, 710, 155, BLACK);
      switch (_graph)
      {
        case 0:
          printAlignText("Temperatura zraka [C]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].temp), _showNData, sizeof(struct sensorData), 10, DATATYPE_FLOAT, GRAPHSTYLE_LINE);
          break;
        case 1:
          printAlignText("Tlak zraka [hPa]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].pressure), _showNData, sizeof(struct sensorData), 10, DATATYPE_FLOAT, GRAPHSTYLE_LINE);
          break;
        case 2:
          printAlignText("Relat. vlaznost zraka [%]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].humidity), _showNData, sizeof(struct sensorData), 10, DATATYPE_FLOAT, GRAPHSTYLE_LINE, 0, 100);
          break;
        case 3:
          printAlignText("eCO2 [ppm]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].eco2), _showNData, sizeof(struct sensorData), 10, DATATYPE_UINT16_T, GRAPHSTYLE_LINE);
          break;
        case 4:
          printAlignText("TVOC [ppb]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].tvoc), _showNData, sizeof(struct sensorData), 10, DATATYPE_UINT16_T, GRAPHSTYLE_LINE);
          break;
        case 5:
          printAlignText("Vodik - H2 [RAW]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].rawH2), _showNData, sizeof(struct sensorData), 10, DATATYPE_UINT16_T, GRAPHSTYLE_LINE);
          break;
        case 6:
          printAlignText("Etanol [RAW]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].rawEthanol), _showNData, sizeof(struct sensorData), 10, DATATYPE_UINT16_T, GRAPHSTYLE_LINE);
          break;
        case 7:
          printAlignText("Baterija [V]", 400, 195, ALIGMENT_CENTERBOT);
          drawGraph(130, 200, 600, 300, &(sdData[(*_dataOffset)].epoch), &(sdData[(*_dataOffset)].battery), _showNData, sizeof(struct sensorData), 10, DATATYPE_FLOAT, GRAPHSTYLE_LINE, 0, 4.4);
          break;
      }
      free(sdData);
    }
  }
}

uint8_t* GUI::weatherIcon(uint8_t i)
{
  switch (i)
  {
    case 1:
      return (uint8_t*)icon01d;
      break;
    case 2:
      return (uint8_t*)icon02d;
      break;
    case 3:
      return (uint8_t*)icon03d;
      break;
    case 4:
      return (uint8_t*)icon04d;
      break;
    case 9:
      return (uint8_t*)icon09d;
      break;
    case 10:
      return (uint8_t*)icon10d;
      break;
    case 11:
      return (uint8_t*)icon11d;
      break;
    case 13:
      return (uint8_t*)icon13d;
      break;
    case 50:
      return (uint8_t*)icon13d;
      break;
    default:
      return (uint8_t*)icon01d;
  }
}

double GUI::map2(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
