#ifndef __GLASSLCD_H__
#define __GLASSLCD_H__

#include "stdint.h"
#include "stm32l0xx_hal.h"
#include "segments.h"

#define PCF85176_ADDR       0x70
#define PCF85176_DISP_ON    0b00001000
#define PCF85176_DISP_OFF   0b00000000
#define PCF85176_1_3_BIAS   0b00000000
#define PCF85176_1_2_BIAS   0b00000100
#define PCF85176_STATIC     0b00000001
#define PCF85176_1_2_MPX    0b00000010
#define PCF85176_1_3_MPX    0b00000011
#define PCF85176_1_4_MPX    0b00000000

// 1:3 LCD Multiplex, 1/3 bias, LCD Enabled
#define LCD_CONFIG    0b01000000 | PCF85176_DISP_ON | PCF85176_1_3_BIAS | PCF85176_1_3_MPX

static const uint8_t asciiToSeg[] = {SPACE, BLANK, QUMARK, BLANK, BLANK, PERC, BLANK, APH, OBRAC, CBRAC, BLANK, BLANK, BLANK, MINUS, BLANK, BLANK, NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK,
                                      LETA, LETB, LETC, LETD, LETE, LETF, LETG, LETH, LETI, LETJ, LETK, LETL, LETM, LETN, LETO, LETP, LETQ, LETR, LETS, LETT, LETU, LETV, LETW, LETX, LETY, LETZ
                                     };

void glassLCD_Begin();
void glassLCD_WriteData(char* s);
void glassLCD_Update();
void glassLCD_Clear();
//void glassLCD_State(uint8_t _state);
//void glassLCD_SetDot(uint8_t _n, uint8_t _dot);
void glassLCD_SetDot(uint8_t _dot);
void glassLCD_WriteArrow(uint8_t _a);
void glassLCD_WriteCmd(uint8_t _comm);

#endif
