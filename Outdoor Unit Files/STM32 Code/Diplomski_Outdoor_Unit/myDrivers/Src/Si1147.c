#include <Si1147.h>

extern I2C_HandleTypeDef hi2c1;

uint8_t Si1147_ReadReg(uint8_t _reg)
{
	uint8_t _ret;
	HAL_I2C_Master_Transmit(&hi2c1, SI1147_ADDR, &_reg, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c1, SI1147_ADDR, &_ret, 1, 1000);
	return _ret;
}

void Si1147_WriteReg(uint8_t _reg, uint8_t _data)
{
	uint8_t _tempData[2] = {_reg, _data};
	HAL_I2C_Master_Transmit(&hi2c1, SI1147_ADDR, _tempData, 2, 1000);
}

void Si1147_WriteRegs(uint8_t *_regs, uint8_t _n)
{
	HAL_I2C_Master_Transmit(&hi2c1, SI1147_ADDR, _regs, _n, 1000);
}

void Si1147_ReadRegs(uint8_t _reg, uint8_t *_data, uint8_t _n)
{
	HAL_I2C_Master_Transmit(&hi2c1, SI1147_ADDR, &_reg, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c1, SI1147_ADDR, _data, _n, 1000);
}

void Si1147_ClearResponseReg()
{
	uint8_t _tempData[2] = {SI1147_COMMAND, 0};
	HAL_I2C_Master_Transmit(&hi2c1, SI1147_ADDR, _tempData, 2, 1000);
}

uint8_t Si1147_GetResponse()
{
	uint8_t tempData = SI1147_RESPONSE;
	HAL_I2C_Master_Transmit(&hi2c1, SI1147_ADDR, &tempData, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c1, SI1147_ADDR, &tempData, 1, 1000);
	return tempData;
}

uint8_t Si1147_Init()
{
	uint8_t tempRegs[2];
	uint8_t _res = 0;

	// INT pin must be used! Especialy while power up, it must be on HIGH logic level!
	//pinMode(_intPin, INPUT_PULLUP);

	// Read PART_ID register (it shound return 0b01000111 for Si1147)
	_res = Si1147_ReadReg(0);
	if (_res != 0b01000111) return 0;

	// Reset IC
	Si1147_WriteReg(SI1147_COMMAND, 0x01);
	_res = Si1147_GetResponse();
	if (_res != 0) return 0;

	// "Unlock IC"
	Si1147_WriteReg(0x07, 0x17);

	// Get the status of Si1147 (0 - sleep, 1 - suspend, 2 - running
	// _res = readReg(SI1147_CHIP_STATUS);
	// if (_res != 2) return false;

	// Disable drive for LED1, LED2 and LED3 (here we are not using proximity feature, just ALS & UV)
	Si1147_WriteReg(SI1147_PARAM_WR, 0);
	tempRegs[0] = SI1147_COMMAND;
	tempRegs[1] = SI1147_PARAM_SET | SI1147_PSLED12_SELECT;
	Si1147_WriteRegs(tempRegs, 2);

	Si1147_WriteReg(SI1147_PARAM_WR, 0);
	tempRegs[0] = SI1147_COMMAND;
	tempRegs[1] = SI1147_PARAM_SET | SI1147_PSLED3_SELECT;
	Si1147_WriteRegs(tempRegs, 2);

	// Set no current @ IR LEDs outputs
	Si1147_WriteReg(SI1147_PS_LED21, 0);
	Si1147_WriteReg(SI1147_PS_LED3, 0);

	// Enable INT on ALS complete
	Si1147_WriteReg(SI1147_IRQ_ENABLE, 1);

	// Set INT on ALS complete
	Si1147_WriteReg(SI1147_IRQ_STATUS, 1);

	// Enable Interrupt on INT pin of Si1147
	Si1147_WriteReg(SI1147_INT_CFG, 1);

	return 1;
}

void Si1147_SetUV()
{
	uint8_t tempRegs[5];

	// Enable UV meas. (1 << 7), ALS IR (1 << 5) and ALS VIS (1 << 4)  in CH list
	Si1147_WriteReg(SI1147_PARAM_WR, (1 << 7) | (1 << 5) | (1 << 4));
	tempRegs[0] = SI1147_COMMAND;
	tempRegs[1] = SI1147_PARAM_SET | SI1147_CHLIST;
	Si1147_WriteRegs(tempRegs, 2);

	// Configure UCOEF
	tempRegs[0] = SI1147_UCOEF0;
	tempRegs[1] = 0x7B;
	tempRegs[2] = 0x6B;
	tempRegs[3] = 0x01;
	tempRegs[4] = 0x00;
	Si1147_WriteRegs(tempRegs, 5);

	// Set the VIS_RANGE and IR_RANGE bits
	Si1147_WriteReg(SI1147_PARAM_WR, 1 << 5);
	//writeReg(SI1147_PARAM_WR, 0);
	tempRegs[0] = SI1147_COMMAND;
	tempRegs[1] = SI1147_PARAM_SET | SI1147_ALS_VIS_ADC_MISC;
	Si1147_WriteRegs(tempRegs, 2);

	Si1147_WriteReg(SI1147_PARAM_WR, 1 << 5);
	//writeReg(SI1147_PARAM_WR, 0);
	tempRegs[0] = SI1147_COMMAND;
	tempRegs[1] = SI1147_PARAM_SET | SI1147_ALS_IR_ADC_MISC;
	Si1147_WriteRegs(tempRegs, 2);
}

void Si1147_ForceUV()
{
	uint32_t _timeout = HAL_GetTick();

	// Force one UV meas.
	Si1147_WriteReg(SI1147_COMMAND, SI1147_ALS_FORCE);

	// Wait for interrupt event
	while ((HAL_GPIO_ReadPin(SI1147_INT_GPIO_Port, SI1147_INT_Pin) == GPIO_PIN_SET) && ((HAL_GetTick() - _timeout) < SI1147_TIMEOUT));

	// Clear interrupt by sending 1 to corresponding interrupt
	Si1147_WriteReg(SI1147_IRQ_STATUS, 1);
}

float Si1147_GetUV()
{
	uint8_t regData[2];

  	// Get the UV data
	Si1147_ReadRegs(SI1147_AUX_DATA0, regData, 2);

	return ((regData[1] << 8) | regData[0]) / 100.0;
}

int16_t Si1147_GetVis()
{
	uint8_t regData[2];

	// Get ambient light visable spectrum data
	Si1147_ReadRegs(SI1147_ALS_VIS_DATA0, regData, 2);
	return (int16_t)(((regData[1] << 8) | regData[0]) - 256);
}

int16_t Si1147_GetIR()
{
	uint8_t regData[2];

	// Get ambient light IR spectrum data
	Si1147_ReadRegs(SI1147_ALS_IR_DATA0, regData, 2);
	return (int16_t)(((regData[1] << 8) | regData[0]) - 256);
}
