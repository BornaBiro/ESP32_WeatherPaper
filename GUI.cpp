#include "GUI.h"

GUI::GUI()
{
    // Empty constructor
}

void GUI::drawMainScreen(Inkplate *_ink, struct sensorData *_sensor, struct currentWeatherHandle *_current, struct forecastListHandle *_forecastList, struct forecastDisplayHandle *_displayForecast, struct tm *_time)
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
    _ink->fillRect(4, 50, 796, 3, BLACK);
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

    int xOffset, xOffsetText;
    for (int i = 1; i < 6; i++) 
    {
        xOffset = i * 160;
        xOffsetText = ((i - 1) * 160) + 20;
        _ink->fillRect(xOffset, 410, 3, 180, BLACK);
        _ink->drawBitmap(xOffset - 105, 420, weatherIcon(atoi(_displayForecast[i - _forecastList->shiftDay].weatherIcon)), 50, 50, BLACK);
        _ink->setTextSize(1);
        _ink->setFont(DISPLAY_FONT_SMALL);
        _ink->setCursor(xOffsetText + 10, 482);
        _ink->print(_displayForecast[i - _forecastList->shiftDay].weatherDesc);
        _ink->setCursor(xOffsetText + 40, 415);
        _ink->print(epochToHuman(_forecastList->forecast[_forecastList->startElement[i - _forecastList->shiftDay]].timestamp).tm_mday);
        _ink->print('.');
        _ink->print(epochToHuman(_forecastList->forecast[_forecastList->startElement[i -_forecastList->shiftDay]].timestamp).tm_mon + 1);
        _ink->setCursor(xOffsetText - 10, 575);
        _ink->print(_displayForecast[i - _forecastList->shiftDay].avgPressure);
        _ink->print("hPa  ");
        _ink->print(_displayForecast[i - _forecastList->shiftDay].avgHumidity);
        _ink->print('%');
        _ink->setCursor(xOffsetText - 10, 588);
        _ink->print(_displayForecast[i - _forecastList->shiftDay].avgWindSpeed, 1);
        _ink->print("m/s ");
        _ink->print(oznakeVjetar[int((_displayForecast[i - _forecastList->shiftDay].avgWindDir / 22.5) + .5) % 16]);
        _ink->print(' ');
        _ink->print(_displayForecast[i - _forecastList->shiftDay].maxWindSpeed, 1);
        _ink->print("m/s");
        _ink->setFont(DISPLAY_FONT);
        _ink->setCursor(xOffsetText, 520);
        _ink->print(_displayForecast[i - _forecastList->shiftDay].maxTemp);
        _ink->setCursor(_ink->getCursorX() + 5, 510);
        _ink->print("o");
        _ink->setCursor(xOffsetText, 555);
        _ink->print(_displayForecast[i - _forecastList->shiftDay].minTemp);
        _ink->setCursor(_ink->getCursorX() + 5, 545);
        _ink->print('o');
        _ink->setCursor(xOffsetText + 20, 400);
        _ink->print(DOW[epochToHuman(_forecastList->forecast[_forecastList->startElement[i - _forecastList->shiftDay]].timestamp).tm_wday]);
    }

    for (int i = 1; i < 3; i++)
    {
        _ink->fillRect(267 * i, 60, 3, 290, BLACK);
    }
    _ink->display();
}

void GUI::drawSelectedDay(Inkplate *_ink, struct forecastListHandle *_forecastList)
{

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