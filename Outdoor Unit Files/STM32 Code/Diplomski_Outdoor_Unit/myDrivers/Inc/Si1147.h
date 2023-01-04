#ifndef __SI1147_H__
#define __SI1147_H__

#include "stdint.h"
#include "stm32l0xx_hal.h"
#include "main.h"

#define SI1147_ADDR						0xC0

// Timeout for reading results im milliseconds
#define SI1147_TIMEOUT					200

#define SI1147_PART_ID                	0x00
#define SI1147_REV_ID                 	0x01
#define SI1147_SEQ_ID                 	0x02
#define SI1147_INT_CFG                	0x03
#define SI1147_IRQ_ENABLE             	0x04
#define SI1147_HW_KEY                 	0x07
#define SI1147_MEAS_RATE0             	0x08
#define SI1147_MEAS_RATE1             	0x09
#define SI1147_PS_LED21               	0x0F
#define SI1147_PS_LED3                	0x10
#define SI1147_UCOEF0                 	0x13
#define SI1147_UCOEF1                 	0x14
#define SI1147_UCOEF2                 	0x15
#define SI1147_UCOEF3                 	0x16
#define SI1147_PARAM_WR               	0x17
#define SI1147_COMMAND                	0x18
#define SI1147_RESPONSE               	0x20
#define SI1147_IRQ_STATUS             	0x21
#define SI1147_ALS_VIS_DATA0          	0x22
#define SI1147_ALS_VIS_DATA1          	0x23
#define SI1147_ALS_IR_DATA0           	0x24
#define SI1147_ALS_IR_DATA1           	0x25
#define SI1147_PS1_DATA0              	0x26
#define SI1147_PS1_DATA1              	0x27
#define SI1147_PS2_DATA0              	0x28
#define SI1147_PS2_DATA1              	0x29
#define SI1147_PS3_DATA0              	0x2A
#define SI1147_PS3_DATA1              	0x2B
#define SI1147_AUX_DATA0              	0x2C
#define SI1147_AUX_DATA1              	0x2D
#define SI1147_PARAM_RD               	0x2E
#define SI1147_CHIP_STATUS            	0x30
#define SI1147_ANA_IN_KEY             	0x3B

// Command Register Summary
#define SI1147_PARAM_QUERY            	0b10000000
#define SI1147_PARAM_SET              	0b10100000
#define SI1147_NOP                    	0b00000000
#define SI1147_RESET                  	0b00000001
#define SI1147_BUSADDR                	0b00000010
#define SI1147_PS_FORCE               	0b00000101
#define SI1147_GET_CAL                	0b00010010
#define SI1147_ALS_FORCE              	0b00000110
#define SI1147_PSALS_FORCE            	0b00000111
#define SI1147_PS_PAUSE               	0b00001001
#define SI1147_ALS_PAUSE              	0b00001010
#define SI1147_PSALS_PAUSE            	0b00001011
#define SI1147_PA_AUTO                	0b00001101
#define SI1147_ALS_AUTO               	0b00001110
#define SI1147_PSALS_AUTO             	0b00001111

/*
   Parameter RAM Summary Table
   Parameters are located in internal memory and are not directly addressable over I2C. They must be indirectly
   accessed using the PARAM_QUERY and PARAM_SET commands described in "4.2. Command Protocol" on
   page 22.
*/
#define SI1147_I2C_ADDR               	0x00
#define SI1147_CHLIST                 	0x01
#define SI1147_PSLED12_SELECT         	0x02
#define SI1147_PSLED3_SELECT          	0x03
#define SI1147_PS_ENCODING            	0x05
#define SI1174_ALS_ENCODING           	0x06
#define SI1147_PS1_ADCMUX             	0x07
#define SI1147_PS2_ADCMUX             	0x08
#define SI1147_PS3_ADCMUX             	0x09
#define SI1147_PS_ADC_COUNTER         	0x0A
#define SI1147_PS_ADC_GAIN            	0x0B
#define SI1147_PS_ADC_MISC            	0x0c
#define SI1147_ALS_IR_ADCMUX          	0x0E
#define SI1147_AUX_ADCMUX             	0x0F
#define SI1147_ALS_VIS_ADC_COUNTER    	0x10
#define SI1147_ALS_VIS_ADC_GAIN       	0x11
#define SI1147_ALS_VIS_ADC_MISC       	0x12
#define SI1147_LED_REC                	0x1C
#define SI1147_ALS_IR_ADC_COUNTER     	0x1D
#define SI1147_ALS_IR_ADC_GAIN        	0x1E
#define SI1147_ALS_IR_ADC_MISC        	0x1F

uint8_t Si1147_ReadReg(uint8_t _reg);
void Si1147_WriteReg(uint8_t _reg, uint8_t _data);
void Si1147_WriteRegs(uint8_t *_regs, uint8_t _n);
void Si1147_ReadRegs(uint8_t _reg, uint8_t *_data, uint8_t _n);
void Si1147_ClearResponseReg();
uint8_t Si1147_GetResponse();
uint8_t Si1147_Init();
void Si1147_SetUV();
void Si1147_ForceUV();
float Si1147_GetUV();
int16_t Si1147_GetVis();
int16_t Si1147_GetIR();

#endif
