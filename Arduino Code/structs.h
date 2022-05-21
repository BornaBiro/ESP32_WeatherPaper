#ifndef __MY_STRUCTS_H__
#define __MY_STRUCTS_H__

#include "stdint.h"

#define SYNC_HEADER     0b00110101
#define DATA1_HEADER    0b00010001
#define DATA2_HEADER    0b00100010

// Struct used for stroring data from sensor (indoor unit)
struct sensorData {
  uint16_t eco2;
  uint16_t tvoc;
  uint16_t rawH2;
  uint16_t rawEthanol;
  uint32_t epoch;
  float battery;
  float temp ;
  float humidity;
  float pressure;
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

struct oneCallApiHandle
{
  char alertEvent[50];
  uint32_t alertStart;
  uint32_t alertEnd;
};

// Structs for RF communication
struct syncStructHandle {
  uint8_t header;
  uint32_t myEpoch;
  uint32_t readInterval;
  uint32_t sendEpoch;
};

struct measruementHandle
{
    float uv;
    int16_t windDir;
    float tempSHT;
    float tempSoil;
    float humidity;
    float pressure;
    float light;
    float windSpeed;
    float rain;
    float battery;
    uint32_t epoch;
    double solarJ;
    double solarW;
};

struct data1StructHandle
{
    uint8_t header;
    uint16_t uv;
    int16_t windDir;
    float tempSHT;
    float tempSoil;
    float humidity;
    float pressure;
    float light;
    float windSpeed;
};

struct data2StructHandle
{
    uint8_t header;
    float rain;
    float battery;
    uint32_t epoch;
    double solarJ;
    double solarW;
};

#endif
