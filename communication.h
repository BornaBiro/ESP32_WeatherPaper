#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "RF24_Inkplate.h"
#include "structs.h"
#include "PCF85063.h"

class communication
{
    public:
    communication();
    void init(RF24_Inkplate * _rf, Inkplate *_dPtr, pcf85063 *_rtcPtr);
    void setupCommunication();
    uint8_t sync(struct syncStructHandle *_s);
    uint8_t getData(struct syncStructHandle *_s, struct data1StructHandle *_d1, struct data2StructHandle *_d2);
    struct syncStructHandle makeSyncStruct();
    time_t newWakeupTime(time_t _current);

    private:
    uint64_t addr[2] = {0x65646F4E31, 0x65646F4E32};
    RF24_Inkplate *_myRf;
    Inkplate *_d;
    pcf85063 *_rtc;
};

#endif