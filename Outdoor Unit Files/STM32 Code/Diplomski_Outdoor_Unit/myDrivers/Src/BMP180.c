#include "BMP180.h"
#include "math.h"

static int16_t AC1,AC2,AC3,VB1,VB2,MB,MC,MD;
static uint16_t AC4,AC5,AC6;
static double c5,c6,mc,md,xx0,xx1,xx2,yy0,yy1,yy2,p0,p1,p2;

uint8_t BMP180_Init()
{
	uint8_t _calData[22];
	double c3,c4,b1;

	// Set data pointer to calibration data
	_calData[0] = 0xAA;
	uint8_t _error = HAL_I2C_Master_Transmit(&hi2c1, BMP180_ADDR, _calData, 1, 1000);

	// Get all calibration data
	_error |= HAL_I2C_Master_Receive(&hi2c1, BMP180_ADDR, _calData, 22, 1000);

	AC1 = (_calData[0] << 8) | _calData[1];
	AC2 = (_calData[2] << 8) | _calData[3];
	AC3 = (_calData[4] << 8) | _calData[5];
	AC4 = (_calData[6] << 8) | _calData[7];
	AC5 = (_calData[8] << 8) | _calData[9];
	AC6 = (_calData[10] << 8) | _calData[11];
	VB1 = (_calData[12] << 8) | _calData[13];
	VB2 = (_calData[14] << 8) | _calData[15];
	MB = (_calData[16] << 8) | _calData[17];
	MC = (_calData[18] << 8) | _calData[19];
	MD = (_calData[20] << 8) | _calData[21];

	c3 = 160.0 * pow(2,-15) * AC3;
	c4 = pow(10,-3) * pow(2,-15) * AC4;
	b1 = pow(160,2) * pow(2,-30) * VB1;
	c5 = (pow(2,-15) / 160) * AC5;
	c6 = AC6;
	mc = (pow(2,11) / pow(160,2)) * MC;
	md = MD / 160.0;
	xx0 = AC1;
	xx1 = 160.0 * pow(2,-13) * AC2;
	xx2 = pow(160,2) * pow(2,-25) * VB2;
	yy0 = c4 * pow(2,15);
	yy1 = c4 * c3;
	yy2 = c4 * b1;
	p0 = (3791.0 - 8.0) / 1600.0;
	p1 = 1.0 - 7357.0 * pow(2,-20);
	p2 = 3038.0 * 100.0 * pow(2,-36);
	return (_error == 0?1:0);
}

float BMP180_ReadTemperatue()
{
	unsigned char _data[2];
	double tu, a;

	// Send request to read temperature
	_data[0] = BMP180_REG_CONTROL;
	_data[1] = BMP180_COMMAND_TEMPERATURE;
	HAL_I2C_Master_Transmit(&hi2c1, BMP180_ADDR, _data, 2, 1000);

	// Wait a little to make temperature measurement
	HAL_Delay(5);

	// Get temp data
	_data[0] = BMP180_REG_RESULT;
	HAL_I2C_Master_Transmit(&hi2c1, BMP180_ADDR, _data, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c1, BMP180_ADDR, _data, 2, 1000);

	// Calculate temp with cal. data
	tu = (_data[0] * 256.0) + _data[1];
	a = c5 * (tu - c6);

	return (a + (mc / (a + md)));
}

float BMP180_ReadPressure()
{
	uint8_t _data[3];
	double pu,s,x,y,z;

	// First you need to read temperature first
	double T = BMP180_ReadTemperatue();

	// Now send request to read pressure with highest resolution
	_data[0] = BMP180_REG_CONTROL;
	_data[1] = BMP180_COMMAND_PRESSURE3;
	HAL_I2C_Master_Transmit(&hi2c1, BMP180_ADDR, _data, 2, 1000);

	// Wait a little to make pressure measurement
	HAL_Delay(26);

	// Get pressure data
	_data[0] = BMP180_REG_RESULT;
	HAL_I2C_Master_Transmit(&hi2c1, BMP180_ADDR, _data, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c1, BMP180_ADDR, _data, 3, 1000);

	// Calculate pressure with cal. data
	pu = (_data[0] * 256.0) + _data[1] + (_data[2]/256.0);
	s = T - 25.0;
	x = (xx2 * pow(s,2)) + (xx1 * s) + xx0;
	y = (yy2 * pow(s,2)) + (yy1 * s) + yy0;
	z = (pu - x) / y;
	return ((p2 * pow(z,2)) + (p1 * z) + p0);
}
