#ifndef __BMP180_H__
#define __BMP180_H__

#include "stdint.h"
#include "stm32l0xx_hal.h"

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;

// Library https://github.com/sparkfun/BMP180_Breakout_Arduino_Library is adopted to work with Atollic
#define BMP180_ADDR 0xEE
#define	BMP180_REG_CONTROL 0xF4
#define	BMP180_REG_RESULT 0xF6
#define	BMP180_COMMAND_TEMPERATURE 0x2E
#define	BMP180_COMMAND_PRESSURE0 0x34
#define	BMP180_COMMAND_PRESSURE1 0x74
#define	BMP180_COMMAND_PRESSURE2 0xB4
#define	BMP180_COMMAND_PRESSURE3 0xF4

uint8_t BMP180_Init();
float BMP180_ReadTemperatue();
float BMP180_ReadPressure();

#endif
