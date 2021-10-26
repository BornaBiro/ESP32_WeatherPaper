#include "GUI.h"

GUI::GUI()
{
    // Empty constructor
}

void GUI::init(Inkplate *_inkPtr)
{
  _ink = _inkPtr;
}

void GUI::drawMainScreen(struct sensorData *_sensor, struct currentWeatherHandle *_current, struct forecastListHandle *_forecastList, struct forecastDisplayHandle *_displayForecast, struct oneCallApiHandle *_one, struct tm *_time)
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
    _ink->drawBitmap(770, 0, refIcon, 30, 30, BLACK);

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

    if (strlen(_one->alertEvent)) _ink->drawBitmap(720, 5, iconWarning, 40, 40, BLACK);

    int xOffset, xOffsetText;
    for (int i = 1; i < 6; i++) 
    {
        xOffset = i * 160;
        xOffsetText = ((i - 1) * 160) + 10;
        _ink->fillRect(xOffset, 410, 3, 180, BLACK);
        _ink->drawBitmap(xOffset - 105, 420, weatherIcon(atoi(_displayForecast[i - _forecastList->shiftDay].weatherIcon)), 50, 50, BLACK);
        _ink->setTextSize(1);
        _ink->setFont(DISPLAY_FONT_SMALL);
        //printStringCenter(_displayForecast[i - _forecastList->shiftDay].weatherDesc, xOffset - 80, 482);
        printAlignText(_displayForecast[i - _forecastList->shiftDay].weatherDesc, xOffset - 80, 482, ALIGMENT_CENTERTOP);
        sprintf(tmp, "%d.%d", epochToHuman(_forecastList->forecast[_forecastList->startElement[i - _forecastList->shiftDay]].timestamp).tm_mday, epochToHuman(_forecastList->forecast[_forecastList->startElement[i -_forecastList->shiftDay]].timestamp).tm_mon + 1);
        //printStringCenter(tmp, xOffset - 80, 415);
        printAlignText(tmp, xOffset - 80, 415, ALIGMENT_CENTERTOP);
        sprintf(tmp, "%d hPa   %d %%", _displayForecast[i - _forecastList->shiftDay].avgPressure, _displayForecast[i - _forecastList->shiftDay].avgHumidity);
        //printStringCenter(tmp, xOffset - 80, 575);
        printAlignText(tmp, xOffset - 80, 575, ALIGMENT_CENTERTOP);
        sprintf(tmp, "%.1f / %.1f m/s   %s", _displayForecast[i - _forecastList->shiftDay].avgWindSpeed, _displayForecast[i - _forecastList->shiftDay].maxWindSpeed, oznakeVjetar[int((_displayForecast[i - _forecastList->shiftDay].avgWindDir / 22.5) + .5) % 16]);
        //printStringCenter(tmp, xOffset - 80, 588);
        printAlignText(tmp, xOffset - 80, 588, ALIGMENT_CENTERTOP);
        _ink->setFont(DISPLAY_FONT);
        sprintf(tmp, "%d | %d", _displayForecast[i - _forecastList->shiftDay].maxTemp, _displayForecast[i - _forecastList->shiftDay].minTemp);
        //printStringCenter(tmp, xOffset - 80, 520);
        printAlignText(tmp, xOffset - 80, 520, ALIGMENT_CENTERTOP);
        //printStringCenter((char*)DOW[epochToHuman(_forecastList->forecast[_forecastList->startElement[i - _forecastList->shiftDay]].timestamp).tm_wday], xOffset - 80, 400);
        printAlignText((char*)DOW[epochToHuman(_forecastList->forecast[_forecastList->startElement[i - _forecastList->shiftDay]].timestamp).tm_wday], xOffset - 80, 400, ALIGMENT_CENTERTOP);
    }

    for (int i = 1; i < 3; i++)
    {
        _ink->fillRect(267 * i, 60, 3, 290, BLACK);
    }
    _ink->display();
}

void GUI::drawSelectedDay(struct forecastListHandle *_forecastList)
{

}

//void GUI::printStringCenter(char *buf, int x, int y)
//{
//    int16_t x1, y1;
//    uint16_t w, h;
//    _ink->getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
//    _ink->setCursor(x - w / 2, y);
//    _ink->print(buf);
//}

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
  float *_intArray;
  _intArray = (float*)malloc(_n * sizeof(float));
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
    char temp[16];
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
    char temp[10];
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