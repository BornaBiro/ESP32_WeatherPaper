#ifndef __E_INK_H__
#define __E_INK_H__
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Adafruit_GFX.h>
#include <Wire.h>
#include "Adafruit_MCP23017.h"
#define E_INK_WIDTH 800
#define E_INK_HEIGHT 600

/*
#define DATA    0xFF000   //GPIO12 - GPIO19 (GPIO12 GPIO13 GPIO14 GPIO15 GPIO16 GPIO17 GPIO18 GPIO19)

#define CL        0x04      //GPIO2
#define CL_SET    {GPIO.out_w1ts = CL;}
#define CL_CLEAR  {GPIO.out_w1tc= CL;}

#define LE        0x10     //GPIO4
#define LE_SET    {GPIO.out_w1ts = LE;}
#define LE_CLEAR  {GPIO.out_w1tc = LE;}

#define OE        0x2000000    //GPIO25
#define OE_SET    {GPIO.out |= OE;}
#define OE_CLEAR  {GPIO.out &= ~OE;}

#define CKV       0x02   //GPIO33
#define CKV_SET   {GPIO.out1_w1ts.val = CKV;}
#define CKV_CLEAR {GPIO.out1_w1tc.val = CKV;}

#define SPH         0x4000000  //GPIO26
#define SPH_SET     {GPIO.out |= SPH;}
#define SPH_CLEAR   {GPIO.out &= ~SPH;}

#define GMODE       0x8000000    //GPIO27
#define GMOD_SET    {GPIO.out |= GMODE;}
#define GMOD_CLEAR  {GPIO.out &= ~GMODE;}

#define SPV         0x01   //GPIO32
#define SPV_SET     {GPIO.out1.val |= SPV;}
#define SPV_CLEAR   {GPIO.out1.val &= ~SPV;}
*/


#define DATA    0x0E8C0030   //D0-D7 = GPIO4 GPIO5 GPIO18 GPIO19 GPIO23 GPIO25 GPIO26 GPIO27

#define CL        0x01    //GPIO0
#define CL_SET    {GPIO.out_w1ts = CL;}
#define CL_CLEAR  {GPIO.out_w1tc = CL;}

#define LE        0x04     //GPIO2
#define LE_SET    {GPIO.out_w1ts = LE;}
#define LE_CLEAR  {GPIO.out_w1tc = LE;}

#define CKV       0x01   //GPIO32
#define CKV_SET   {GPIO.out1_w1ts.val = CKV;}
#define CKV_CLEAR {GPIO.out1_w1tc.val = CKV;}

#define SPH         0x02   //GPIO33
#define SPH_SET     {GPIO.out1_w1ts.val = SPH;}
#define SPH_CLEAR   {GPIO.out1_w1tc.val = SPH;}

//I/O Expander - A Channel
#define GMOD       1    //GPIOA1
#define GMOD_SET    {mcp.digitalWrite(GMOD, HIGH);}
#define GMOD_CLEAR  {mcp.digitalWrite(GMOD, HIGH);}

#define OE        	0    //GPIOA0
#define OE_SET    	{mcp.digitalWrite(OE, HIGH);}
#define OE_CLEAR  	{mcp.digitalWrite(OE, LOW);}

#define SPV         2   //GPIOA5
#define SPV_SET     {mcp.digitalWrite(SPV, HIGH);}
#define SPV_CLEAR   {mcp.digitalWrite(SPV, LOW);}

//#define BOOST        8   //GPIOA2
//#define BOOST_SET    {mcp.digitalWrite(BOOST, HIGH);}
//#define BOOST_CLEAR  {mcp.digitalWrite(BOOST, LOW);}

//#define VPOS         8   //GPIOA3
//#define VPOS_SET     {mcp.digitalWrite(VPOS, HIGH);}
//#define VPOS_CLEAR   {mcp.digitalWrite(VPOS, LOW);}

//#define VNEG         8   //GPIOA4
//#define VNEG_SET     {mcp.digitalWrite(VNEG, HIGH);}
//#define VNEG_CLEAR   {mcp.digitalWrite(VNEG, LOW);}

//#define VCOM         8   //GPIOA6
//#define VCOM_SET     {mcp.digitalWrite(VCOM, HIGH);}
//#define VCOM_CLEAR   {mcp.digitalWrite(VCOM, LOW);}

#define WAKEUP         3   //GPIOA3
#define WAKEUP_SET     {mcp.digitalWrite(WAKEUP, HIGH);}
#define WAKEUP_CLEAR   {mcp.digitalWrite(WAKEUP, LOW);}

#define PWRUP         4   //GPIOA4
#define PWRUP_SET     {mcp.digitalWrite(PWRUP, HIGH);}
#define PWRUP_CLEAR   {mcp.digitalWrite(PWRUP, LOW);}

#define VCOM         5   //GPIOA6
#define VCOM_SET     {mcp.digitalWrite(VCOM, HIGH);}
#define VCOM_CLEAR   {mcp.digitalWrite(VCOM, LOW);}

#define GDISP_SCREEN_HEIGHT 600
#define CKV_CLOCK ckvClock();
#define SCLOCK        {  CL_SET; CL_CLEAR; }

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

#define REF_RATE    5
#define BLACK 1
#define WHITE 0

static void ckvClock();
static void usleep1();

class eink : public Adafruit_GFX {
 public:
 	eink();
 	//uint8_t D_memory[100][600];
	//uint8_t* D_memory;
	uint8_t* D_memory_new;
	//uint8_t LUT[16] = {B00000000, B00000001, B00000100, B00000101, B00010000, B00010001, B00010100, B00010101, B01000000, B01000001, B01000100, B01000101, B01010000, B01010001, B01010100, B01010101};
	uint8_t LUT[16] = {B11111111, B11111101, B11110111, B11110101, B11011111, B11011101, B11010111, B11010101, B01111111, B01111101, B01110111, B01110101, B01011111, B01011101, B01010111, B01010101};
	uint8_t LUT2[16] = {B10101010, B10101001, B10100110, B10100101, B10011010, B10011001, B10010110, B10010101, B01101010, B01101001, B01100110, B01100101, B01011010, B01011001, B01010110, B01010101};
	uint8_t LUTW[16] = {B00000000, B00000010, B00001000, B00001010, B00100000, B00100010, B00101000, B00101010, B10000000, B10000010, B10001000, B10001010, B10100000, B10100010, B10101000, B10101010};
	uint8_t pixelMaskLUT[8] = {B10000000, B01000000, B00100000, B00010000, B00001000, B00000100, B00000010, B00000001};
	
	void drawPixel(int16_t x0, int16_t y0, uint16_t color);
  	void begin(void);
  	void clearDisplay();
  	void display();
  	void draw_mode_off();
  	void draw_mode_on();
  	void advance_line();
  	void begin_frame();
  	void end_frame();
  	void begin_line();
  	void end_line();
  	void clean(uint8_t c);
	void cleanFast(uint8_t c);
  	void setRotation(uint8_t);
  	void einkOff(void);
  	void einkOn(void);
	void end_line_slow();
  	
  	private:
  		uint8_t _rotation = 0;
  		uint16_t _tempRotation;

};

#endif
