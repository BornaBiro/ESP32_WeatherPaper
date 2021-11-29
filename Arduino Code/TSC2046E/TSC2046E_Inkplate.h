#ifndef __TSC2046E_INKPLATE_H__
#define __TSC2046E_INKPLATE_H__

#include "Arduino.h"
#include "SPI.h"
#include "Inkplate.h"

class TSC2046E_Inkplate
{
  public:
    TSC2046E_Inkplate();
    void begin(SPIClass *_s, Inkplate * _i, uint8_t _cs, uint8_t _irq);
    void calibrate(uint16_t _xMin, uint16_t _xMax, uint16_t _yMin, uint16_t _yMax, uint16_t _xScreenMin, uint16_t _xScreenMax, uint16_t _yScreenMin, uint16_t _yScreenMax);
    uint16_t getX();
    uint16_t getY();
    uint16_t getZ1();
    uint16_t getZ2();
    uint16_t getP(uint16_t _x);
    float getBatteryVoltage();
    uint8_t available(int *_x, int *_y, int *_p = NULL);
    
  private:
    SPIClass *_mySpi = NULL;
    Inkplate *_ink = NULL;
    uint8_t _irqPin;
    uint8_t _csPin;
    uint16_t _xPlateRes = 650;
    uint8_t _samples = 16;
    uint16_t _xMin = 0;
    uint16_t _xMax = 4093;
    uint16_t _yMin = 0;
    uint16_t _yMax = 4093;
    uint16_t _xScreenMin = 0;
    uint16_t _xScreenMax = 4093;
    uint16_t _yScreenMin = 0;
    uint16_t _yScreenMax = 4093;
};

#endif