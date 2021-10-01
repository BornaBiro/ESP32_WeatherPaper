#ifndef __MY_STRUCTS_H__
#define __MY_STRUCTS_H__

#include "stdint.h"

// Struct used for stroring data from sensor (indoor unit)
struct sensorData {
  float temp = 0;
  float humidity = 0;
  float pressure = 0;
  float co2 = 0;
  float voc = 0;
  uint32_t timeStamp = 0;
};

// Struct for current weather conditions
struct currentWeatherHandle
{
  char weatherDesc[50];
  char weatherIcon[4];
  char city[20];
  uint8_t humidity;
  uint8_t clouds;
  uint16_t weatherId;
  uint16_t windDir;
  uint16_t pressure;
  uint16_t pressureGnd;
  uint16_t visibility;
  int16_t timezone;
  float temp;
  float feelsLike;
  float windSpeed;
  float windGust;
  float rain;
  float snow;
  uint32_t timestamp;
  uint32_t sunrise;
  uint32_t sunset;
};

// Struct for one element of 5 days/3h forecast
struct forecastWeatherHandle
{
  char weatherDesc[50];
  char weatherIcon[4];
  uint8_t clouds;
  uint8_t humidity;
  uint8_t probability;
  uint16_t weatherId;
  uint16_t pressureGnd;
  uint16_t visibility;
  uint16_t pressure;
  uint16_t windDir;
  float temp;
  float maxTemp;
  float minTemp;
  float feelsLike;
  float windSpeed;
  float windGust;
  float rain;
  float snow;
  uint32_t timestamp;
};

// Struct for 5 days/3h forecast
struct forecastListHandle
{
  uint8_t startElement[7];
  uint8_t numberOfData;
  uint8_t shiftDay;
  struct forecastWeatherHandle forecast[45];
};

// Struct for forcast display
struct forecastDisplayHandle
{
  int8_t maxTemp;
  int8_t minTemp;
  uint16_t avgHumidity;
  uint16_t avgPressure;
  float avgWindSpeed;
  float maxWindSpeed;
  float avgWindDir;
  char *weatherDesc;
  char *weatherIcon;
};

#endif
