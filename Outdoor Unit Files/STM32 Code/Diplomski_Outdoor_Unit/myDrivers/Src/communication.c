#include "communication.h"

uint8_t rfBuffer[32];
uint64_t addr[2] = {0x65646F4E31, 0x65646F4E32};
const char syncStr[] = {"SYNC %3d"};

void communication_Setup()
{
    RF24_setAutoAck(1);
    RF24_enableAckPayload();
    RF24_setChannel(0);
    RF24_setDataRate(RF24_250KBPS);
    RF24_setPALevel(RF24_PA_MAX, 1);
    RF24_openWritingPipe(addr[0]);
    RF24_openReadingPipe(1, addr[1]);
    RF24_stopListening();
}

//uint8_t communication_Sync(struct syncStructHandle *_s)
//{
//    uint8_t syncOk = 0;
//    uint8_t syncTimeout = 180;
//    uint32_t time1 = HAL_GetTick();
//    while (!syncOk && syncTimeout != 0)
//    {
//        if ((HAL_GetTick() - time1) > 1000)
//        {
//            time1 = HAL_GetTick();
//            char lcdTemp[9];
//            sprintf(lcdTemp, syncStr, syncTimeout--);
//            glassLCD_Clear();
//            glassLCD_WriteData(lcdTemp);
//            glassLCD_Update();
//
//            _s->myEpoch = RTC_GetTime();
//            RF24_write(_s, sizeof(struct syncStructHandle), 0);
//            if (RF24_isAckPayloadAvailable())
//            {
//                syncOk = 1;
//                while (RF24_available(NULL))
//                {
//                    RF24_read(rfBuffer, 32);
//                }
//                if (rfBuffer[0] == SYNC_HEADER)
//                {
//                    memcpy(_s, rfBuffer, sizeof(struct syncStructHandle));
//                    syncOk = 1;
//                }
//            }
//        }
//    }
//    return syncOk;
//}

uint8_t communication_Transmit(void* _transmitBuffer, uint8_t _txSize, uint8_t* _receiveBuffer)
{
    uint8_t _succ = 0;
    RF24_write(_transmitBuffer, _txSize, 0);
    if (RF24_isAckPayloadAvailable())
    {
        while (RF24_available(NULL))
        {
            RF24_read(_receiveBuffer, 32);
        }
        _succ = 1;
    }
    return _succ;
}
