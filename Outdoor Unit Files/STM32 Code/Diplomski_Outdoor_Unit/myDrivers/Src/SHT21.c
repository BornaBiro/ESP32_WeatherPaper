#include "SHT21.h"

extern I2C_HandleTypeDef hi2c1;

// It doesn't init the sensor, it used to verify is sensor connected to the I2C
uint8_t SHT21_Init()
{
  uint8_t dummy = 0;
  uint8_t _err = HAL_I2C_Master_Transmit(&hi2c1, SHT21_ADDRESS, &dummy, 0, 1000);
  return (_err == 0?1:0);
}

float SHT21_ReadHumidity() {
	// Start the measurment and get the data
	uint16_t _rh = SHT21_ReadRegister(SHT21_TRIG_HUM_MEAS_NO_HOLD);

	// Clean last two bits
	_rh &= ~0x0003;

	// Return relative humidity multiplied by ten to avoid using float;
  	return (-6.0 + 125.0/65536 * (float)_rh);
}

float SHT21_ReadTemperature()
{
	// Start the measurment and get the data
	uint16_t _t = SHT21_ReadRegister(SHT21_TRIG_TEMP_MEAS_NO_HOLD);

	// Clean last two bits
	_t &= ~0x0003;

	// Return relative humidity multiplied by ten to avoid using float;
  	return (-46.85 + 175.72/65536 * (float)_t);
}

uint16_t SHT21_ReadRegister(uint8_t _reg)
{
	uint8_t _data[3];

	// Use No Hold Master Mode - No Clock Streching!
	HAL_I2C_Master_Transmit(&hi2c1, SHT21_ADDRESS, &_reg, 1, 1000);

	// Wait for measurment to be completed
	HAL_Delay(100);

	// Read the data
	HAL_I2C_Master_Receive(&hi2c1, SHT21_ADDRESS, _data, 3, 1000);

	// Pack it!
	return (uint16_t)(_data[0] << 8) | _data[1];
}
