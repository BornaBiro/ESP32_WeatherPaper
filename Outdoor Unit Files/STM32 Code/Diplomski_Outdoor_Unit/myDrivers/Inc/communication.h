#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "stdint.h"
#include "stm32l0xx_hal.h"

#include "RF24.h"
#include "rtc.h"
#include "glassLCD.h"
#include "myStructs.h"

void communication_Setup();
//uint8_t communication_Sync(struct syncStructHandle *_s);
uint8_t communication_Transmit(void* _transmitBuffer, uint8_t _txSize, uint8_t* _receiveBuffer);
#endif
