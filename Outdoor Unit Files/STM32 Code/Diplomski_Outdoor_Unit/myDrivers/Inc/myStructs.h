#ifndef _MY_STRUCTS_H_
#define _MY_STRUCTS_H_

struct syncStructHandle
{
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

struct  data1StructHandle
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
