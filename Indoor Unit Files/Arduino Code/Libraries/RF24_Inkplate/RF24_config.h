
/*
 Copyright (C)
    2011            J. Coliz <maniacbug@ymail.com>
    2015-2019 TMRh20
    2015            spaniakos <spaniakos@gmail.com>
    2015            nerdralph
    2015            zador-blood-stained
    2016            akatran
    2017-2019 Avamander <avamander@gmail.com>
    2019            IkpeohaGodson

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
*/

#ifndef __RF24_CONFIG_H__
#define __RF24_CONFIG_H__

/*** USER DEFINES:    ***/
#define FAILURE_HANDLING

/**
 * User access to internally used delay time (in microseconds) during RF24::powerUp()
 * @warning This default value compensates for all supported hardware. Only adjust this if you
 * know your radio's hardware is, in fact, genuine and reliable.
 */
#if !defined(RF24_POWERUP_DELAY)
#define RF24_POWERUP_DELAY	5000
#endif

/**********************/
#define rf24_max(a, b) (a>b?a:b)
#define rf24_min(a, b) (a<b?a:b)

#define RF24_SPI_SPEED 10000000

#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static SPIClass *_SPI;
        
#ifndef _BV
    #define _BV(x) (1<<(x))
#endif

    // Progmem is Arduino-specific
    // Arduino DUE is arm and does not include avr/pgmspace
    #if defined (ARDUINO_ARCH_ESP8266) || defined (ESP32)
        #include <pgmspace.h>
        #define PRIPSTR "%s"
        #ifndef pgm_read_ptr
          #define pgm_read_ptr(p) (*(p))
        #endif
    #elif defined (ARDUINO) && !defined (ESP_PLATFORM) && ! defined (__arm__) && !defined (__ARDUINO_X86__) || defined (XMEGA)
        #include <avr/pgmspace.h>
        #define PRIPSTR "%S"

    #else // !defined (ARDUINO) || defined (ESP_PLATFORM) || defined (__arm__) || defined (__ARDUINO_X86__) && !defined (XMEGA)
        #if !defined (ARDUINO) // This doesn't work on Arduino DUE
            typedef char const char;

        #else // Fill in pgm_read_byte that is used, but missing from DUE
          #ifdef ARDUINO_ARCH_AVR
            #include <avr/pgmspace.h>
          #endif
          #ifndef pgm_read_byte
            #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
          #endif
        #endif // !defined (ARDUINO)

        #ifndef prog_uint16_t
          typedef uint16_t prog_uint16_t;
        #endif
        #ifndef PSTR
          #define PSTR(x) (x)
        #endif
        #ifndef printf_P
          #define printf_P printf
        #endif
        #ifndef strlen_P
          #define strlen_P strlen
        #endif
        #ifndef PROGMEM
          #define PROGMEM
        #endif
        #ifndef pgm_read_word
          #define pgm_read_word(p) (*(p))
        #endif
        #if !defined pgm_read_ptr || defined ARDUINO_ARCH_MBED
          #define pgm_read_ptr(p) (*(p))
        #endif
        #ifndef PRIPSTR
          #define PRIPSTR "%s"
        #endif

    #endif // !defined (ARDUINO) || defined (ESP_PLATFORM) || defined (__arm__) || defined (__ARDUINO_X86__) && !defined (XMEGA)

#if defined (SPI_HAS_TRANSACTION) && !defined (SPI_UART) && !defined (SOFTSPI)
    #define RF24_SPI_TRANSACTIONS
#endif // defined (SPI_HAS_TRANSACTION) && !defined (SPI_UART) && !defined (SOFTSPI)

#endif // __RF24_CONFIG_H__
