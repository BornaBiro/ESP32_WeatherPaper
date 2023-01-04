#ifndef __SLEEP_H__
#define __SLEEP_H__

#include "stdint.h"
#include "stm32l0xx_hal.h"

// List of all periph. handlers that should be disabled in sleep
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi1;
extern ADC_HandleTypeDef hadc;

extern void SystemClock_Config();

void Sleep_LightSleep();

#endif
