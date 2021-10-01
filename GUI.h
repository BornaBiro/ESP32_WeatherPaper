#ifndef __GUI_H__
#define __GUI_H__

#include "Inkplate.h"
#include "structs.h"
#include "icons.h"
#include "time.h"
#include "Neucha_Regular17pt7b.h"
#include "RobotoCondensed_Regular6pt7b.h"
#include "timeAndDate.h"

#define DISPLAY_FONT &Neucha_Regular17pt7b
#define DISPLAY_FONT_SMALL &RobotoCondensed_Regular6pt7b

static const char* oznakeVjetar[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};

class GUI
{
    public:
    GUI();
    void init(Inkplate *_inkPtr);
    void drawMainScreen(struct sensorData *_sensor, struct currentWeatherHandle *_current, struct forecastListHandle *_forecastList, struct forecastDisplayHandle *_displayForecast, struct tm *_time);
    void drawSelectedDay(struct forecastListHandle *_forecastList);
    void printStringCenter(char *buf, int x, int y);
    uint8_t* weatherIcon(uint8_t i);

    private:
    Inkplate *_ink;
};

#endif