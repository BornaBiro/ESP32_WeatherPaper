#include "TSC2046E.h"

TSC2046E::TSC2046E()
{
    
}

void TSC2046E::begin(SPIClass *_s, uint8_t _cs, uint8_t _irq)
{
    _mySpi = _s;
    _csPin = _cs;
    _irqPin = _irq;
    pinMode(_csPin, OUTPUT);
    pinMode(_irqPin, INPUT_PULLUP);
}

void TSC2046E::calibrate(uint16_t __xMin, uint16_t __xMax, uint16_t __yMin, uint16_t __yMax, uint16_t __xScreenMin, uint16_t __xScreenMax, uint16_t __yScreenMin, uint16_t __yScreenMax)
{
    _xMin = __xMin;
    _xMax = __xMax;
    _yMin = __yMin;
    _yMax = __yMax;
    _xScreenMin = __xScreenMin;
    _xScreenMax = __xScreenMax;
    _yScreenMin = __yScreenMin;
    _yScreenMax = __yScreenMax;
    
}

uint16_t TSC2046E::getX()
{
    digitalWrite(_csPin, LOW);
    // 2MHz clock -> 500kHz/16 = 31.250kHz Sample Rate
    _mySpi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    // 0b1, 101 - x measure, 0 - 12 bits,  0 - differential meas., 00 - low power between meas.
    _mySpi->transfer(0b11010000);
    delayMicroseconds(50);
    int _x = ((_mySpi->transfer(0x00) << 8 | _mySpi->transfer(0x00)) >> 3);
    _mySpi->endTransaction();
    digitalWrite(_csPin, HIGH);
    return _x;
}

uint16_t TSC2046E::getY()
{
    digitalWrite(_csPin, LOW);
    // 2MHz clock -> 500kHz/16 = 31.250kHz Sample Rate
    _mySpi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    // 0b1, 001 - y measure, 0 - 12 bits,  0 - differential meas., 00 - low power between meas.
    _mySpi->transfer(0b10010000);
    delayMicroseconds(50);
    int _x = ((_mySpi->transfer(0x00) << 8 | _mySpi->transfer(0x00)) >> 3);
    _mySpi->endTransaction();
    digitalWrite(_csPin, HIGH);
    return _x;
}

uint16_t TSC2046E::getZ1()
{
    digitalWrite(_csPin, LOW);
    // 2MHz clock -> 500kHz/16 = 31.250kHz Sample Rate
    _mySpi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    // 0b1, 011 - z1 measure, 0 - 12 bits,  0 - differential meas., 00 - low power between meas.
    _mySpi->transfer(0b10110000);
    delayMicroseconds(50);
    int _x = ((_mySpi->transfer(0x00) << 8 | _mySpi->transfer(0x00)) >> 3);
    _mySpi->endTransaction();
    digitalWrite(_csPin, HIGH);
    return _x;
}

uint16_t TSC2046E::getZ2()
{
    digitalWrite(_csPin, LOW);
    // 2MHz clock -> 500kHz/16 = 31.250kHz Sample Rate
    _mySpi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    // 0b1, 100 - z2 measure, 0 - 12 bits,  0 - differential meas., 00 - low power between meas.
    _mySpi->transfer(0b11000000);
    delayMicroseconds(50);
    int _x = ((_mySpi->transfer(0x00) << 8 | _mySpi->transfer(0x00)) >> 3);
    _mySpi->endTransaction();
    digitalWrite(_csPin, HIGH);
    return _x;
}

uint16_t TSC2046E::getP(uint16_t _x)
{
    uint32_t _z1 = 0;
    uint32_t _z2 = 0;
    for (int i = 0; i < _samples; i++)
    {
        _z1 += getZ1();
        _z2 += getZ2();
    }
    _z1 /= _samples;
    _z2 /= _samples;
    delay(1);
    return (uint16_t)(_xPlateRes * (_x / 4096.0) * (((_z2 * 1.0) / _z1) - 1));
}

uint8_t TSC2046E::available(int *_x, int *_y, int *_p)
{
    if (digitalRead(_irqPin)) return 0;
    delay(1);
    uint32_t _tempX = 0;
    uint32_t _tempY = 0;
    for (int i = 0; i < _samples; i++)
    {
        _tempX += getX();
        _tempY += getY();
    }
    _tempX /= _samples;
    _tempY /= _samples;
    int _tempP = getP(_tempX);
    if (_tempP < 700)
    {
        _tempX = map(_tempX, _xMin, _xMax, _xScreenMin, _xScreenMax);
        _tempY = map(_tempY, _yMin, _yMax, _yScreenMin, _yScreenMax);
        if (_tempX <= _xScreenMax && _tempX >= _xScreenMin && _tempY <= _yScreenMax && _tempY >= _yScreenMin)
        {
            *_x = (uint16_t)_tempX;
            *_y = (uint16_t)_tempY;
            if (_p != NULL) *_p = _tempP;
            return 1;
        }
    }
    return 0;
}