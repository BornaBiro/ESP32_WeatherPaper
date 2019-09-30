//#include <avr/pgmspace.h>
#include <stdlib.h>

#include "Adafruit_GFX.h"
#include "E_INK.h"
Adafruit_MCP23017 mcp;


eink::eink() : Adafruit_GFX(E_INK_WIDTH, E_INK_HEIGHT) {
}

void eink::begin(void) {
  //  GPIO.out_w1tc = DATA | CL | LE | OE | GMODE | SPH;
  //  GPIO.out1_w1tc.val = SPV | CKV;
  Wire.begin();
  mcp.begin(0);
  //Wire.setClock(100000);
  //mcp.pinMode(VPOS, OUTPUT); //V_POS Enable
  //mcp.pinMode(VNEG, OUTPUT); //V_NEG Enable
  //mcp.pinMode(BOOST, OUTPUT); //BOOST Enable
  mcp.pinMode(VCOM, OUTPUT);
  mcp.pinMode(PWRUP, OUTPUT);
  mcp.pinMode(WAKEUP, OUTPUT);

  //einkOn();

  //CONTROL PINS
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);
  mcp.pinMode(OE, OUTPUT);
  mcp.pinMode(GMOD, OUTPUT);
  mcp.pinMode(SPV, OUTPUT);

  //DATA PINS
  pinMode(4, OUTPUT); //D0
  pinMode(5, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT); //D7


  //  Wire.beginTransmission(0x48);
  //  Wire.write(0x04);
  //  Wire.write(B00000000);
  //  Wire.endTransmission();

  //D_memory = (uint8_t*)malloc(600*104);
  //if(D_memory == NULL) {
  //	do{
  //		delay(100);
  //	}while(true);
  //}

  D_memory_new = (uint8_t*)malloc(600 * 104);
  if (D_memory_new == NULL) {
    do {
      delay(100);
    } while (true);
  }
  //memset(D_memory, 0, 62400);
  memset(D_memory_new, 0, 62400);
}

void eink::clearDisplay() {
  //for(int i = 0; i<100; i++) {
  //	for(int j = 0; j<600; j++) {
  //		D_memory[i][j] = 0;
  //	}
  //}
  memset(D_memory_new, 0, 62400);
}

void eink::draw_mode_on() {
  SPH_SET; //dsph_gpio::set::set(1 << DSPH_BIT);
  SPV_SET;
  delay(1);
  OE_SET;
  delay(1);
  GMOD_SET;//dgmode_gpio::set::set(1 << DGMODE_BIT);
  CKV_SET; //dckv_gpio::set::set(1 << DCKV_BIT);
  //for _ in 0..20 {
  //    usleep(10000);
  //}
  delayMicroseconds(400);
}

void eink::draw_mode_off() {
  CKV_CLOCK;
  GMOD_CLEAR; //dgmode_gpio::clr::set(1 << DGMODE_BIT);
  CKV_CLOCK;
  GPIO.out &= ~DATA; //dd_gpio::clr::set(0xFF << DD_SHIFT);
  CL_CLEAR; //dcl_gpio::clr::set(1 << DCL_BIT);
  LE_CLEAR; //dle_gpio::clr::set(1 << DLE_BIT);
  OE_CLEAR; //doe_gpio::clr::set(1 << DOE_BIT);
  SPV_CLEAR; //dspv_gpio::clr::set(1 << DSPV_BIT);
  CKV_CLEAR; //dckv_gpio::clr::set(1 << DCKV_BIT);
  SPH_CLEAR; //dsph_gpio::clr::set(1 << DSPH_BIT);
}

void eink::advance_line() {
  CKV_CLEAR; //dckv_gpio::clr::set(1 << DCKV_BIT);
  usleep1();
  CKV_SET; //dckv_gpio::set::set(1 << DCKV_BIT);
  usleep1();
}

void eink::begin_frame() {
  //OE_SET;
  //SPV_SET; //dspv_gpio::set::set(1 << DSPV_BIT);
  delayMicroseconds(400); //usleep(500);
  SPV_CLEAR; //dspv_gpio::clr::set(1 << DSPV_BIT);
  usleep1();
  CKV_CLEAR; //dckv_gpio::clr::set(1 << DCKV_BIT);
  delayMicroseconds(20); //usleep(25);
  CKV_SET; //dckv_gpio::set::set(1 << DCKV_BIT);
  usleep1();
  SPV_SET; //dspv_gpio::set::set(1 << DSPV_BIT);
  delayMicroseconds(20); //usleep(25);
  // For some reason I have to advance 3 times to fill the whole screen
  advance_line();
  advance_line();
  advance_line();

}

void eink::end_frame() {
  //delayMicroseconds(2000);
}

void eink::begin_line() {
  // This line is only required if you need a long time to calculate each scanline (1 of 2)
  //OE_SET; //doe_gpio::set::set(1 << DOE_BIT); //
  SPH_CLEAR; //dsph_gpio::clr::set(1 << DSPH_BIT);
  //usleep1();
}

void eink::end_line() {

  SPH_SET; //dsph_gpio::set::set(1 << DSPH_BIT);
  //usleep1();

  CKV_CLEAR; //dckv_gpio::clr::set(1 << DCKV_BIT);
  CL_SET; //dcl_gpio::set::set(1 << DCL_BIT);
  usleep1();
  //usleep1();
  //usleep1();
  //usleep1();
  //usleep1();
  //usleep1();
  CL_CLEAR; //dcl_gpio::clr::set(1 << DCL_BIT);
  //usleep1();
  CKV_SET; //dckv_gpio::set::set(1 << DCKV_BIT);
  usleep1();
  //usleep1();
  //OE_CLEAR; //doe_gpio::clr::set(1 << DOE_BIT);
  // This time controls the load and therefore might induce a hearable frequency
  // on the inductors of a switching power supply.
  //usleep1(); //
  //OE_SET; //doe_gpio::set::set(1 << DOE_BIT);
  //usleep1();
  LE_SET; //dle_gpio::set::set(1 << DLE_BIT);
  //usleep1();
  LE_CLEAR; //dle_gpio::clr::set(1 << DLE_BIT);
  //usleep1();

  // This line is only required if you need a long time to calculate each scanline (2 of 2)
  //OE_CLEAR; //doe_gpio::clr::set(1 << DOE_BIT); //
}

void eink::end_line_slow() {

  SPH_SET; //dsph_gpio::set::set(1 << DSPH_BIT);
  //usleep1();

  CKV_CLEAR; //dckv_gpio::clr::set(1 << DCKV_BIT);
  CL_SET; //dcl_gpio::set::set(1 << DCL_BIT);
  delayMicroseconds(20);
  //usleep1();
  CL_CLEAR; //dcl_gpio::clr::set(1 << DCL_BIT);
  CKV_SET; //dckv_gpio::set::set(1 << DCKV_BIT);
  delayMicroseconds(4);
  //OE_CLEAR; //doe_gpio::clr::set(1 << DOE_BIT);
  // This time controls the load and therefore might induce a hearable frequency
  // on the inductors of a switching power supply.
  //usleep1(); //
  //OE_SET; //doe_gpio::set::set(1 << DOE_BIT);
  //usleep1();
  LE_SET; //dle_gpio::set::set(1 << DLE_BIT);
  //usleep1();
  LE_CLEAR; //dle_gpio::clr::set(1 << DLE_BIT);
  //usleep1();

  // This line is only required if you need a long time to calculate each scanline (2 of 2)
  //OE_CLEAR; //doe_gpio::clr::set(1 << DOE_BIT); //
}

void usleep1() {
  int z = 32;
  while (z--) {
    asm("NOP");
  };
}

void eink::drawPixel(int16_t x0, int16_t y0, uint16_t color) {
  if (x0 > 799 || y0 > 599 || x0 < 0 || y0 < 0) return;
  switch (_rotation) {
    case 1:
      _swap_int16_t(x0, y0);
      x0 = _width - x0 - 1;
      break;
    case 2:
      x0 = _width - x0 - 1;
      y0 = _height - y0 - 1;
      break;
    case 3:
      _swap_int16_t(x0, y0);
      y0 = _width - y0 - 1;
      break;
  }
  int x = x0 / 8;
  int x_sub = x0 % 8;
  uint8_t temp = D_memory_new[104 * y0 + x];
  //D_memory[x][y0] = ~pixelMaskLUT[x_sub] & temp | (color ? pixelMaskLUT[x_sub] : 0) ;
  D_memory_new[104 * y0 + x] = ~pixelMaskLUT[x_sub] & temp | (color ? pixelMaskLUT[x_sub] : 0) ;
}

void eink::display() {
  //enable();

  //CONTROL PINS
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);
  mcp.pinMode(OE, OUTPUT);
  mcp.pinMode(GMOD, OUTPUT);
  mcp.pinMode(SPV, OUTPUT);

  //DATA PINS
  pinMode(4, OUTPUT); //D0
  pinMode(5, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT); //D7
  einkOn();
  uint16_t _pos;
  uint32_t _send;
  uint8_t data;
  draw_mode_on();
  SPV_SET; //dspv_gpio::set::set(1 << DSPV_BIT);
  delayMicroseconds(500); //usleep(500);

  for (int i = 0; i < 6; i++) {
    cleanFast(1);
    delayMicroseconds(500);
  }
  //cleanFast(0);

  for (int i = 0; i < 6; i++) {
    cleanFast(0);
    delayMicroseconds(500);
  }

  for (int i = 0; i < 7; i++) {
    cleanFast(1);
    delayMicroseconds(500);
  }
  cleanFast(2);
  /*
  	for(int i = 0; i<REF_RATE; i++) {
  		cleanFast(1);
  	}
  	for(int i = 0; i<REF_RATE; i++) {
  	  cleanFast(0);
  	}

  	//for(int i = 0; i<REF_RATE; i++) {
  	//  cleanFast(1);
  	//}
  	//draw_mode_off();
  	/*
    for (int k = 0; k < REF_RATE; k++) {
  	begin_frame();
  	for (int i = 0; i < 600; i++) {
  	begin_line();
  	for (int j = 0; j < 100; j++) {
  		GPIO.out &= ~DATA;
  		GPIO.out |= LUT2[~(D_memory[j][i] >> 4) & 0x0F]<<12;
  		CL_SET;
  		CL_CLEAR;
  		GPIO.out &= ~DATA;
  		GPIO.out |= LUT2[~(D_memory[j][i]) & 0x0F] << 12;
  		CL_SET;
  		CL_CLEAR;
  	}
  	end_line();
  	}
  	end_frame();
    }
  */
  /*
    for (int k = 0; k < REF_RATE; k++) {
    begin_frame();
    for (int i = 0; i < 600; i++) {
    begin_line();
    for (int j = 0; j < 100; j++) {
  	GPIO.out &= ~DATA;
  	GPIO.out |= LUT2[(D_memory[j][i] >> 4)] << 12;
  	CL_SET;
  	CL_CLEAR;
  	GPIO.out &= ~DATA;
  	GPIO.out |= LUT2[(D_memory[j][i]) & 0x0F] << 12;
  	CL_SET;
  	CL_CLEAR;
    }
    end_line();
    }
    end_frame();
    }
  */
  //draw_mode_on();
  //SPV_SET; //dspv_gpio::set::set(1 << DSPV_BIT);
  //delayMicroseconds(470); //usleep(500);
  /*
  	for (int k = 0; k < REF_RATE; k++) {
  		begin_frame();
  		for (int i = 0; i < 600; i++) {
  			begin_line();
  			for (int j = 0; j < 100; j++) {
  				GPIO.out &= ~DATA;
  				GPIO.out |= LUTW[~(D_memory_new[i*100+j] >> 4) & 0x0F]<<12;
  				CL_SET;
  				CL_CLEAR;
  				GPIO.out &= ~DATA;
  				GPIO.out |= LUTW[~(D_memory_new[i*100+j]) & 0x0F] << 12;
  				CL_SET;
  				CL_CLEAR;
  			}
  			end_line();
  		}
  		end_frame();
  	}
  */

  /*
  	for (int k = 0; k < REF_RATE; k++) {
  		begin_frame();
  		for (int i = 0; i < 600; i++) {
  			begin_line();
  			for (int j = 0; j < 104; j++) {
  				_pos = i*104+j;
  				//GPIO.out &= ~DATA;
  				//GPIO.out |= LUT2[~(D_memory_new[_pos] >> 4) & 0x0F]<<12;
  				//CL_SET;
  				//CL_CLEAR;
  				//GPIO.out &= ~DATA;
  				//GPIO.out |= LUT2[~(D_memory_new[_pos]) & 0x0F] << 12;
  				//CL_SET;
  				//CL_CLEAR;
  				GPIO.out_w1tc = DATA | CL;
  				GPIO.out_w1ts = (LUT2[~(D_memory_new[_pos] >> 4) & 0x0F]<<12);
  				GPIO.out_w1ts = CL;
  				GPIO.out_w1tc = DATA | CL;
  				GPIO.out_w1ts = (LUT2[~(D_memory_new[_pos]) & 0x0F] << 12);
  				GPIO.out_w1ts = CL;
  			}
  			end_line();
  		}
  		end_frame();
  	}
  	//cleanFast(2);
  */
  /*
    for (int k = 0; k < REF_RATE-2; k++) {
  	begin_frame();
  	for (int i = 0; i < 600; i++) {
  		begin_line();
  		for (int j = 0; j < 104; j++) {
  			_pos = i*104+j;
  			//GPIO.out &= ~DATA;
  			//GPIO.out |= LUT[(D_memory_new[_pos] >> 4)] << 12;
  			//CL_SET;
  			//CL_CLEAR;
  			//GPIO.out &= ~DATA;
  			//GPIO.out |= LUT[(D_memory_new[_pos]) & 0x0F] << 12;
  			//CL_SET;
  			//CL_CLEAR;
  			data = LUT[(D_memory_new[_pos] >> 4)];
  			_send = ((data&B00000011)<<4) | (((data&B00001100)>>2)<<18) | (((data&B00010000)>>4)<<23) | (((data&B11100000)>>5)<<25);
  			GPIO.out_w1tc = DATA | CL;
  			GPIO.out_w1ts = (_send);
  			GPIO.out_w1ts = CL;
  			data = LUT[(D_memory_new[_pos]) & 0x0F];
  			_send = ((data&B00000011)<<4) | (((data&B00001100)>>2)<<18) | (((data&B00010000)>>4)<<23) | (((data&B11100000)>>5)<<25);
  			GPIO.out_w1tc = DATA | CL;
  			GPIO.out_w1ts = (_send);
  			GPIO.out_w1ts = CL;
  		}
  		end_line();
  	}
  	end_frame();
  	delayMicroseconds(1500);
    }
    /*

    for (int k = 0; k < REF_RATE; k++) {
  	begin_frame();
  	for (int i = 0; i < 600; i++) {
  		begin_line();
  		for (int j = 0; j < 104; j++) {
  			_pos = i*104+j;
  			//GPIO.out &= ~DATA;
  			//GPIO.out |= LUT2[(D_memory_new[_pos] >> 4)] << 12;
  			//CL_SET;
  			//CL_CLEAR;
  			//GPIO.out &= ~DATA;
  			//GPIO.out |= LUT2[(D_memory_new[_pos]) & 0x0F] << 12;
  			//CL_SET;
  			//CL_CLEAR;
  			data = ~LUT2[(D_memory_new[_pos] >> 4)];
  			_send = ((data&B00000011)<<4) | (((data&B00001100)>>2)<<18) | (((data&B00010000)>>4)<<23) | (((data&B11100000)>>5)<<25);
  			GPIO.out_w1tc = DATA | CL;
  			GPIO.out_w1ts = (_send);
  			GPIO.out_w1ts = CL;
  			data = ~LUT2[(D_memory_new[_pos]) & 0x0F];
  			_send = ((data&B00000011)<<4) | (((data&B00001100)>>2)<<18) | (((data&B00010000)>>4)<<23) | (((data&B11100000)>>5)<<25);
  			GPIO.out_w1tc = DATA | CL;
  			GPIO.out_w1ts = _send;
  			GPIO.out_w1ts = CL;
  		}
  		end_line();
  	}
  	end_frame();
  	delayMicroseconds(1500);
    }

    for (int k = 0; k < 2; k++) {
  	begin_frame();
  	for (int i = 0; i < 600; i++) {
  		begin_line();
  		for (int j = 0; j < 104; j++) {
  			_pos = i*104+j;
  			//GPIO.out &= ~DATA;
  			//GPIO.out |= LUT2[(D_memory_new[_pos] >> 4)] << 12;
  			//CL_SET;
  			//CL_CLEAR;
  			//GPIO.out &= ~DATA;
  			//GPIO.out |= LUT2[(D_memory_new[_pos]) & 0x0F] << 12;
  			//CL_SET;
  			//CL_CLEAR;
  			data = LUT[(D_memory_new[_pos] >> 4)];
  			_send = ((data&B00000011)<<4) | (((data&B00001100)>>2)<<18) | (((data&B00010000)>>4)<<23) | (((data&B11100000)>>5)<<25);
  			GPIO.out_w1tc = DATA | CL;
  			GPIO.out_w1ts = (_send);
  			GPIO.out_w1ts = CL;
  			data = LUT[(D_memory_new[_pos]) & 0x0F];
  			_send = ((data&B00000011)<<4) | (((data&B00001100)>>2)<<18) | (((data&B00010000)>>4)<<23) | (((data&B11100000)>>5)<<25);
  			GPIO.out_w1tc = DATA | CL;
  			GPIO.out_w1ts = _send;
  			GPIO.out_w1ts = CL;
  		}
  		end_line();
  	}
  	end_frame();
  	//delayMicroseconds(1500);
    }
  */
  for (int k = 0; k < 7; k++) {
    begin_frame();
    for (int i = 0; i < 600; i++) {
      begin_line();
      for (int j = 0; j < 104; j++) {
        _pos = i * 104 + j;
        //GPIO.out &= ~DATA;
        //GPIO.out |= LUT2[(D_memory_new[_pos] >> 4)] << 12;
        //CL_SET;
        //CL_CLEAR;
        //GPIO.out &= ~DATA;
        //GPIO.out |= LUT2[(D_memory_new[_pos]) & 0x0F] << 12;
        //CL_SET;
        //CL_CLEAR;
        data = LUT2[(D_memory_new[_pos] >> 4)];
        _send = ((data & B00000011) << 4) | (((data & B00001100) >> 2) << 18) | (((data & B00010000) >> 4) << 23) | (((data & B11100000) >> 5) << 25);
        GPIO.out_w1tc = DATA | CL;
        GPIO.out_w1ts = (_send);
        GPIO.out_w1ts = CL;
        data = LUT2[(D_memory_new[_pos]) & 0x0F];
        _send = ((data & B00000011) << 4) | (((data & B00001100) >> 2) << 18) | (((data & B00010000) >> 4) << 23) | (((data & B11100000) >> 5) << 25);
        GPIO.out_w1tc = DATA | CL;
        GPIO.out_w1ts = _send;
        GPIO.out_w1ts = CL;
      }
      end_line();
    }
    end_frame();
    delayMicroseconds(500);
  }
  /*
    begin_frame();
    for (int i = 0; i < 600; i++) {
  	begin_line();
  	for (int j = 0; j < 100; j++) {
  		GPIO.out &= ~DATA;
  		GPIO.out |= LUTW[~(D_memory_new[i*100+j] >> 4) & 0x0F]<<12;
  		CL_SET;
  		CL_CLEAR;
  		GPIO.out &= ~DATA;
  		GPIO.out |= LUTW[~(D_memory_new[i*100+j]) & 0x0F] << 12;
  		CL_SET;
  		CL_CLEAR;
  	}
  	end_line();
    }
    end_frame();
  */
  cleanFast(2);
  draw_mode_off();
  //memcpy(D_memory, D_memory_new, 62400);

  //CONTROL PINS
  pinMode(0, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(32, INPUT_PULLUP);
  pinMode(33, INPUT_PULLUP);
  mcp.pinMode(OE, INPUT);
  mcp.pullUp(OE, HIGH);
  mcp.pinMode(GMOD, INPUT);
  mcp.pullUp(GMOD, HIGH);
  mcp.pinMode(SPV, INPUT);
  mcp.pullUp(SPV, HIGH);

  //DATA PINS
  pinMode(4, INPUT_PULLUP); //D0
  pinMode(5, INPUT_PULLUP);
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  pinMode(23, INPUT_PULLUP);
  pinMode(25, INPUT_PULLUP);
  pinMode(26, INPUT_PULLUP);
  pinMode(27, INPUT_PULLUP); //D7
}

void ckvClock() {
  CKV_CLEAR;
  usleep1();
  CKV_SET;
  usleep1();
}

void eink::clean(uint8_t c) {
  uint8_t data = c == 0 ? B10101010 : B01010101;
  uint32_t _send = ((data & B00000011) << 4) | (((data & B00001100) >> 2) << 18) | (((data & B00010000) >> 4) << 23) | (((data & B11100000) >> 5) << 25);
  // enable();
  draw_mode_on();
  SPV_SET; //dspv_gpio::set::set(1 << DSPV_BIT);
  delayMicroseconds(500); //usleep(500);
  for (int k = 0; k < REF_RATE; k++) {
    begin_frame();
    for (int i = 0; i < 600; i++) {

      begin_line();
      for (int j = 0; j < 100; j++) {
        GPIO.out &= ~DATA;
        GPIO.out |= _send;
        CL_SET;
        CL_CLEAR;
        GPIO.out &= ~DATA;
        GPIO.out |= _send;
        CL_SET;
        CL_CLEAR;
      }
      end_line();
    }
    end_frame();
  }
  draw_mode_off();
}

void eink::cleanFast(uint8_t c) {
  uint8_t data; // = c == 0 ? B10101010 : c==1?B01010101: B00000000;
  if (c == 0) {
    data = B10101010;
  } else if (c == 1) {
    data = B01010101;
  } else if (c == 2) {
    data = B00000000;
  }
  uint32_t _send = ((data & B00000011) << 4) | (((data & B00001100) >> 2) << 18) | (((data & B00010000) >> 4) << 23) | (((data & B11100000) >> 5) << 25);
  // enable();
  //draw_mode_on();
  //SPV_SET; //dspv_gpio::set::set(1 << DSPV_BIT);
  //delayMicroseconds(500); //usleep(500);
  begin_frame();
  for (int i = 0; i < 600; i++) {

    begin_line();
    for (int j = 0; j < 104; j++) {
      //GPIO.out &= ~DATA;
      //GPIO.out |= data << 12;
      //CL_SET;
      //CL_CLEAR;
      //GPIO.out &= ~DATA;
      //GPIO.out |= data << 12;
      //CL_SET;
      //CL_CLEAR;
      GPIO.out_w1tc = DATA | CL;
      GPIO.out_w1ts = (_send);
      GPIO.out_w1ts = CL;
      GPIO.out_w1tc = DATA | CL;
      GPIO.out_w1ts = (_send);
      GPIO.out_w1ts = CL;
    }
    end_line();
    //end_line_slow();
  }
  end_frame();
  //draw_mode_off();
}

void eink::einkOn() {
  //BOOST_CLEAR;	//digitalWrite(21, LOW);
  //delay(1);
  //VNEG_SET;		//digitalWrite(5, HIGH);
  //delay(1);
  //VPOS_SET;		//digitalWrite(23, HIGH);
  //delay(1);
  //VCOM_CLEAR;
  //delay(1);

  WAKEUP_SET;
  //Enable all rails
  Wire.beginTransmission(0x48);
  Wire.write(0x01);
  Wire.write(B00111111);
  Wire.endTransmission();
  //Set out voltage on LDO outputs
  Wire.beginTransmission(0x48);
  Wire.write(0x02);
  Wire.write(B00100011);
  Wire.endTransmission();
  //Set VCOM Voltage
  Wire.beginTransmission(0x48);
  Wire.write(0x03);
  Wire.write(192);
  Wire.endTransmission();
  //Set power up times (all on 3mS)
  Wire.beginTransmission(0x48);
  Wire.write(0x0A);
  Wire.write(0);
  Wire.endTransmission();
  //Set Power Down Seq.
  Wire.beginTransmission(0x48);
  Wire.write(0x0B);
  Wire.write(B00011011);
  Wire.endTransmission();
  //Set Power Down Times (all on 6mS)
  Wire.beginTransmission(0x48);
  Wire.write(0x0C);
  Wire.write(0);
  Wire.endTransmission();

  VCOM_SET;
  PWRUP_SET;
}

void eink::einkOff() {
  GPIO.out &= ~(DATA | CL | LE);
  SPH_CLEAR;
  OE_CLEAR;
  GMOD_CLEAR;
  SPV_CLEAR;

  PWRUP_CLEAR;
  //VCOM_CLEAR;

  Wire.beginTransmission(0x48);
  Wire.write(0x01);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(100);
  //WAKEUP_CLEAR;
  //VCOM_SET;
  //VPOS_CLEAR;	//digitalWrite(23, LOW);
  //delay(1);
  //VNEG_CLEAR;	//digitalWrite(5, LOW);
  //delay(1);
  //BOOST_SET;		//digitalWrite(21, HIGH);
  //delay(1);
}

void eink::setRotation(uint8_t r) {
  _rotation = r % 4;
  switch (rotation) {
    case 0:
      _width  = E_INK_WIDTH;
      _height = E_INK_HEIGHT;
      break;
    case 1:
      _width  = E_INK_HEIGHT;
      _height = E_INK_WIDTH;
      break;
    case 2:
      _width  = E_INK_WIDTH;
      _height = E_INK_HEIGHT;
      break;
    case 3:
      _width  = E_INK_HEIGHT;
      _height = E_INK_WIDTH;
      break;
  }
}
