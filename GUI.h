#ifndef __GUI_H__
#define __GUI_H__

#include "Inkplate.h"
#include "structs.h"
#include "icons.h"
#include "time.h"
#include "Neucha_Regular17pt7b.h"
#include "RobotoCondensed_Regular6pt7b.h"
#include "timeAndDate.h"
#include "math.h"

#define DISPLAY_FONT &Neucha_Regular17pt7b
#define DISPLAY_FONT_SMALL &RobotoCondensed_Regular6pt7b

enum graphDataType
{
  DATATYPE_FLOAT,
  DATATYPE_DOUBLE,
  DATATYPE_UINT8_T,
  DATATYPE_UINT16_T,
  DATATYPE_INT16_T,
  DATATYPE_INT,
};

enum graphStyle
{
  GRAPHSTYLE_LINE,
  GRAPHSTYLE_DOT,
  GRAPHSTYLE_COLUMN
};

enum alignment
{
  ALIGMENT_LEFTBOT,
  ALIGMENT_CENTERBOT,
  ALIGMENT_RIGHTBOT,
  ALIGMENT_LEFT,
  ALIGMENT_CENTER,
  ALIGMENT_RIGHT,
  ALIGMENT_LEFTTOP,
  ALIGMENT_CENTERTOP,
  ALIGMENT_RIGHTTOP
};

static const char* oznakeVjetar[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};

class GUI
{
    public:
    GUI();
    void init(Inkplate *_inkPtr);
    void drawMainScreen(struct sensorData *_sensor, struct currentWeatherHandle *_current, struct forecastListHandle *_forecastList, struct forecastDisplayHandle *_displayForecast, struct oneCallApiHandle *_one, struct data1StructHandle *_d1, struct tm *_time);
    void drawSelectedDay(struct forecastListHandle *_forecastList);
    //void printStringCenter(char *buf, int x, int y);
    void printAlignText(char *text, int16_t x, int16_t y, enum alignment align);
    void drawGraph(int16_t _x, int16_t _y, uint16_t _w, uint16_t _h, void *_xData, void *_yData, uint8_t _n, uint8_t _step, uint8_t _m, enum graphDataType _dataType, graphStyle _style, float _min = sqrt(-1), float _max = sqrt(-1));
    uint8_t* weatherIcon(uint8_t i);
    double map2(double x, double in_min, double in_max, double out_min, double out_max);
    private:
    Inkplate *_ink;
};

#endif