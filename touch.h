#ifndef _TOUCH_CPP_
#define _TOUCH_CPP_

#include <Wire.h>
#include "Adafruit_MCP23017.h"
#define X0 11
#define Y0 10
#define X1 9
#define Y1 8
#define AX 34
#define AY 35


void initTouch();

bool touchAvailable();

bool readTouch(int *t);
#endif
