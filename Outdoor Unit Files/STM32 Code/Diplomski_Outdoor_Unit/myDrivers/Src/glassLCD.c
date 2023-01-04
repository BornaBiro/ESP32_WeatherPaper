#include "glassLCD.h"
#include "string.h"

extern I2C_HandleTypeDef hi2c1;
static uint8_t _lcdTemp[9] = {0};
static uint8_t _dots = 0;

void glassLCD_Begin()
{
	glassLCD_WriteCmd(LCD_CONFIG);
	glassLCD_Clear();
	glassLCD_Update();
}

void glassLCD_WriteData(char* s)
{
  // Clear everything
  glassLCD_Clear();

	// Get the size of string
	uint8_t _n = strlen(s);

	//Convert ASCII to segment data and save it to buffer
	for (int i = 0; i < _n; i++)
	{
		_lcdTemp[i] |= asciiToSeg[s[i] - ' '];
	}
}

void glassLCD_Update()
{
	uint8_t _dotMask = 0;

	// Buffer for I2C data
	uint8_t _data[9] = {0};

	//Write segments
	_data[0] = 0;
	for (int i = 0; i < 8; i++)
	{
		_data[i + 1] = _lcdTemp[i];
	}
	HAL_I2C_Master_Transmit(&hi2c1, PCF85176_ADDR, _data, 9, 1000);

	//Now write dots
	for (int i = 0; i < 8; i++)
	{
		_dotMask = (_dots & (1 << (7 - i))) ? 0b00100000 : 0b00000000;
	    _data[0] = 2 + (3 * i);
	    _data[1] = ((_lcdTemp[i] & 3) << 6) | _dotMask | (_lcdTemp[i + 1] >> 3);
	    HAL_I2C_Master_Transmit(&hi2c1, PCF85176_ADDR, _data, 2, 1000);
	}
}

void glassLCD_Clear()
{
	memset(_lcdTemp, 0, 8);
	_dots = 0;
}

//void glassLCD_State(uint8_t _state)
//{
//	glassLCD_WriteCmd((LCD_CONFIG) & ((_state & 1) << 3));
//}

//void glassLCD_SetDot(uint8_t _n, uint8_t _dot)
//{
//	_n &= 7;
//	if (_dot)
//	{
//		_dots |= 1 << _n;
//	}
//	else
//	{
//		_dots &= ~(1 << _n);
//	}
//}

void glassLCD_SetDot(uint8_t _dot)
{
	_dots = _dot;
}

void glassLCD_WriteArrow(uint8_t _a)
{
  for (int i = 0; i < 8; i++)
  {
	  if (_a & 1 << (7 - i))
  	  {
	  	  _lcdTemp[i] |= SEGW;
  	  }
  	  else
  	  {
	  	  _lcdTemp[i] &= ~(SEGW);
  	  }
  }
}

void glassLCD_WriteCmd(uint8_t _comm)
{
	_comm = _comm | 0b10000000;
	HAL_I2C_Master_Transmit(&hi2c1, PCF85176_ADDR, &_comm, 1, 1000);
}
