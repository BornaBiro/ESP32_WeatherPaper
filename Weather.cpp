#include "Weather.h"

OWMWeather::OWMWeather()
{
  //Empty constructor (for now)
}

uint8_t OWMWeather::getCurrentWeather(const char* _url, struct currentWeatherHandle *_c)
{
  WiFiClient client;
  HTTPClient http;
  DynamicJsonDocument doc(25000);
  http.useHTTP10(true);

  if (http.begin(client, _url))
  {
    if (http.GET() > 0)
    {
      deserializeJson(doc, http.getStream());
      if (doc["cod"] == 200)
      {
        strncpy(_c->weatherIcon, doc["weather"][0]["icon"], sizeof(_c->weatherIcon) - 1);
        removeCroLetters(strncpy(_c->weatherDesc, doc["weather"][0]["description"], sizeof(_c->weatherDesc) - 1));
        removeCroLetters(strncpy(_c->city, doc["name"], sizeof(_c->city) - 1));
        _c->temp = doc["main"]["temp"];
        _c->feelsLike = doc["main"]["feels_like"];
        _c->humidity = doc["main"]["humidity"];
        _c->pressure = doc["main"]["pressure"];
        _c->pressureGnd = doc["main"]["grnd_level"];
        _c->windSpeed = doc["wind"]["speed"];
        _c->windGust = doc["wind"]["gust"];
        _c->windDir = doc["wind"]["deg"];
        _c->visibility = doc["visibility"];
        _c->clouds = doc["clouds"]["all"];
        _c->rain = doc["rain"]["1h"];
        _c->snow = doc["snow"]["1h"];
        _c->timezone = doc["timezone"];
        _c->timestamp = doc["dt"];
        _c->sunrise = doc["sys"]["sunrise"];
        _c->sunrise += _c->timezone;
        _c->sunset = doc["sys"]["sunset"];
        _c->sunset += _c->timezone;
      }
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
  http.end();
  return 1;
}

uint8_t OWMWeather::getForecastWeather(const char* _url, struct forecastListHandle *_f, struct forecastDisplayHandle *_d)
{
  WiFiClient client;
  HTTPClient http;
  DynamicJsonDocument doc(24576);
  http.useHTTP10(true);

  if (http.begin(client, _url))
  {
    if (http.GET() > 0)
    {
      deserializeJson(doc, http.getStream());
      if (atoi(doc["cod"]) == 200)
      {
        _f->numberOfData = doc["cnt"];
        for (int i = 0; i < _f->numberOfData; i++)
        {
          removeCroLetters(strncpy(_f->forecast[i].weatherDesc, doc["list"][i]["weather"][0]["description"], sizeof(_f->forecast[i].weatherDesc) - 1));
          strncpy(_f->forecast[i].weatherIcon, doc["list"][i]["weather"][0]["icon"], sizeof(_f->forecast[i].weatherIcon) - 1);
          _f->forecast[i].clouds = doc["list"][i]["clouds"]["all"];
          _f->forecast[i].humidity = doc["list"][i]["main"]["humidity"];
          _f->forecast[i].probability = (float)(doc["list"][i]["pop"]) * 100;
          _f->forecast[i].weatherId = doc["list"][i]["weather"][0]["id"];
          _f->forecast[i].pressureGnd = doc["list"][i]["main"]["grnd_level"];
          _f->forecast[i].visibility = doc["list"][i]["visibility"];
          _f->forecast[i].pressure = doc["list"][i]["main"]["pressure"];
          _f->forecast[i].windDir = doc["list"][i]["wind"]["deg"];
          _f->forecast[i].minTemp = doc["list"][i]["main"]["temp_min"];
          _f->forecast[i].temp = doc["list"][i]["main"]["temp"];
          _f->forecast[i].maxTemp = doc["list"][i]["main"]["temp_max"];
          _f->forecast[i].feelsLike = doc["list"][i]["main"]["feels_like"];
          _f->forecast[i].windSpeed = doc["list"][i]["wind"]["speed"];
          _f->forecast[i].windGust = doc["list"][i]["wind"]["gust"];
          _f->forecast[i].rain = doc["list"][i]["rain"]["3h"];
          _f->forecast[i].snow = doc["list"][i]["snow"]["3h"];
          _f->forecast[i].timestamp = doc["list"][i]["dt"];
        }

        // Group data into "days"
        // Skip first element
        int n = 1;
        _f->startElement[0] = 0;
        for (int i = 1; i < _f->numberOfData; i++)
        {
          if (epochToHuman(_f->forecast[i].timestamp).tm_hour == 0)
          {
            _f->startElement[n] = i;
            n++;
          }
        }
        _f->startElement[n] = _f->numberOfData;

        // Calculate avg and max data for forecast display
        memset(_d, 0, sizeof(forecastDisplayHandle) * 7);
        for (int i = 0; i < 6; i++)
        {
          int nElements = _f->startElement[i + 1] - _f->startElement[i];
          _d[i].maxTemp = _f->forecast[_f->startElement[i]].maxTemp;
          _d[i].minTemp = _f->forecast[_f->startElement[i]].minTemp;
          _d[i].maxWindSpeed = _f->forecast[_f->startElement[i]].windGust;
          //Serial.print("Start element:");
          //Serial.print(_f->startElement[i], DEC);
          //Serial.print(" end element:");
          //Serial.println(_f->startElement[i + 1], DEC);
          int something = 0;
          float eastWestVectSum = 0;
          float northSouthVectSum = 0;
          for (int j = _f->startElement[i]; j < _f->startElement[i + 1]; j++)
          {
            _d[i].avgPressure += _f->forecast[j].pressureGnd;
            _d[i].avgHumidity += _f->forecast[j].humidity;
            //_d[i].avgWindSpeed += _f->forecast[j].windSpeed;
            eastWestVectSum += _f->forecast[j].windSpeed * sin(_f->forecast[j].windDir * PI / 180);
            northSouthVectSum += _f->forecast[j].windSpeed * cos(_f->forecast[j].windDir * PI / 180);
            if (_d[i].maxTemp < _f->forecast[j].maxTemp) _d[i].maxTemp = round(_f->forecast[j].maxTemp);
            if (_d[i].minTemp > _f->forecast[j].minTemp) _d[i].minTemp = round(_f->forecast[j].minTemp);
            if (_d[i].maxWindSpeed < _f->forecast[j].windGust) _d[i].maxWindSpeed = _f->forecast[j].windGust;
            if (epochToHuman(_f->forecast[j].timestamp).tm_hour == 15)
            {
              _d[i].weatherIcon = _f->forecast[j].weatherIcon;
              _d[i].weatherDesc = _f->forecast[j].weatherDesc;
            }
            //Serial.print(i, DEC);
            //Serial.print('/');
            //Serial.println(j, DEC);
            something++;
          }
          Serial.println(something, DEC);
          _d[i].avgPressure /= nElements;
          _d[i].avgHumidity /= nElements;
          eastWestVectSum /= nElements;
          northSouthVectSum /= nElements;
          _d[i].avgWindSpeed = sqrt(eastWestVectSum * eastWestVectSum + northSouthVectSum * northSouthVectSum);
          _d[i].avgWindDir = atan(eastWestVectSum / northSouthVectSum) * 180 / PI;
          _d[i].avgWindDir = (_d[i].avgWindDir >= 0?_d[i].avgWindDir:_d[i].avgWindDir + 360);
          Serial.println(_d[i].avgWindDir);
          //_d[i].avgWindSpeed /= nElements;
        }
        _f->shiftDay = 1;
        if (_f->startElement[1] - _f->startElement[0] < 3) _f->shiftDay = 0;
      }
    }
    http.end();
  }
}

void OWMWeather::removeCroLetters(char *p)
{
  int webTextSize = strlen(p);
  for (int16_t i = 0; i < webTextSize; i++)
  {
    if (p[i] == 196 || p[i] == 197) memmove(&p[i], &p[i + 1], webTextSize - i);
    if (p[i] == 141 || p[i] == 135) p[i] = 'c';
    if (p[i] == 161) p[i] = 's';
    if (p[i] == 190) p[i] = 'z';
  }
}
