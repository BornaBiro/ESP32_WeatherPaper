#ifndef __WEATHER_H___
#define __WEATHER_H___

#include "structs.h"
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "time.h"

class OWMWeather
{
    public:
    OWMWeather();
    uint8_t getCurrentWeather(const char* _url, struct currentWeatherHandle *_c);
    uint8_t getForecastWeather(const char* _url, struct forecastListHandle *_f, struct forecastDisplayHandle *_d);
    void removeCroLetters(char *p);
    struct tm epochToHuman(time_t _t);
};

#endif
