/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * @file RF24.h
 *
 * Class declaration for RF24 and helper enums
 */

#ifndef __RF24_H__
#define __RF24_H__

#include "RF24_config.h"
#include <string.h>
#include "stm32l0xx_hal.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;

/**
 * @defgroup PALevel Power Amplifier level
 * Power Amplifier level. The units dBm (decibel-milliwatts or dB<sub>mW</sub>)
 * represents a logarithmic signal loss.
 * @see RF24::setPALevel()
 * @see RF24::getPALevel()
 * @{
 */
typedef enum {
    /**
     * (0) represents:
     * nRF24L01 | Si24R1 with<br>lnaEnabled = 1 | Si24R1 with<br>lnaEnabled = 0
     * :-------:|:-----------------------------:|:----------------------------:
     *  -18 dBm | -6 dBm | -12 dBm
     */
    RF24_PA_MIN = 0,
    /**
     * (1) represents:
     * nRF24L01 | Si24R1 with<br>lnaEnabled = 1 | Si24R1 with<br>lnaEnabled = 0
     * :-------:|:-----------------------------:|:----------------------------:
     *  -12 dBm | 0 dBm | -4 dBm
     */
    RF24_PA_LOW,
    /**
     * (2) represents:
     * nRF24L01 | Si24R1 with<br>lnaEnabled = 1 | Si24R1 with<br>lnaEnabled = 0
     * :-------:|:-----------------------------:|:----------------------------:
     *  -6 dBm | 3 dBm | 1 dBm
     */
    RF24_PA_HIGH,
    /**
     * (3) represents:
     * nRF24L01 | Si24R1 with<br>lnaEnabled = 1 | Si24R1 with<br>lnaEnabled = 0
     * :-------:|:-----------------------------:|:----------------------------:
     *  0 dBm | 7 dBm | 4 dBm
     */
    RF24_PA_MAX,
    /**
     * (4) This should not be used and remains for backward compatibility.
     */
    RF24_PA_ERROR
} rf24_pa_dbm_e;

/**
 * @}
 * @defgroup Datarate datarate
 * How fast data moves through the air. Units are in bits per second (bps).
 * @see RF24::setDataRate()
 * @see RF24::getDataRate()
 * @{
 */
typedef enum {
    /** (0) represents 1 Mbps */
    RF24_1MBPS = 0,
    /** (1) represents 2 Mbps */
    RF24_2MBPS,
    /** (2) represents 250 kbps */
    RF24_250KBPS
} rf24_datarate_e;

/**
 * @}
 * @defgroup CRCLength CRC length
 * The length of a CRC checksum that is used (if any).<br>Cyclical Redundancy
 * Checking (CRC) is commonly used to ensure data integrity.
 * @see RF24::setCRCLength()
 * @see RF24::getCRCLength()
 * @see RF24::disableCRC()
 * @{
 */
typedef enum {
    /** (0) represents no CRC checksum is used */
    RF24_CRC_DISABLED = 0,
    /** (1) represents CRC 8 bit checksum is used */
    RF24_CRC_8,
    /** (2) represents CRC 16 bit checksum is used */
    RF24_CRC_16
} rf24_crclength_e;


//uint16_t ce_pin; /**< "Chip Enable" pin, activates the RX or TX role */
//uint16_t csn_pin; /**< SPI Chip select */
uint16_t ce_pin;
uint16_t csn_pin;
GPIO_TypeDef* ce_port;
GPIO_TypeDef* csn_port;
uint8_t status; /** The status byte returned from every SPI transaction */
uint8_t payload_size; /**< Fixed size of payloads */
uint8_t dynamic_payloads_enabled; /**< Whether dynamic payloads are enabled. */
uint8_t ack_payloads_enabled; /**< Whether ack payloads are enabled. */
uint8_t pipe0_reading_address[5]; /**< Last address set on pipe 0 for reading. */
uint8_t addr_width; /**< The address width to use - 3,4 or 5 bytes. */
uint8_t config_reg; /**< For storing the value of the NRF_CONFIG register */
uint8_t _is_p_variant; /** For storing the result of testing the toggleFeatures() affect */

void RF24_beginTransaction();
void RF24_endTransaction();

void RF24_init(GPIO_TypeDef* _ce_port, uint16_t _ce_pin, GPIO_TypeDef* _cs_port, uint16_t _cs_pin);
uint8_t RF24_begin();
uint8_t RF24_isChipConnected();
void RF24_startListening(void);
void RF24_stopListening(void);
//uint8_t RF24_available(void);
void RF24_read(void* buf, uint8_t len);
//uint8_t RF24_write(const void* buf, uint8_t len);
//void RF24_openWritingPipe(const uint8_t* address);
//void RF24_openReadingPipe(uint8_t number, const uint8_t* address);
uint8_t RF24_available(uint8_t* pipe_num);
uint8_t RF24_rxFifoFull();
void RF24_powerDown(void);
void RF24_powerUp(void);
uint8_t RF24_write(const void* buf, uint8_t len, const uint8_t multicast);
//uint8_t RF24_writeFast(const void* buf, uint8_t len);
uint8_t RF24_writeFast(const void* buf, uint8_t len, const uint8_t multicast);
uint8_t RF24_writeBlocking(const void* buf, uint8_t len, uint32_t timeout);
uint8_t RF24_txStandBy();
//uint8_t RF24_txStandBy(uint32_t timeout, uint8_t startTx = 0);
uint8_t RF24_txStandByT(uint32_t timeout, uint8_t startTx);
uint8_t RF24_writeAckPayload(uint8_t pipe, const void* buf, uint8_t len);
void RF24_whatHappened(uint8_t *tx_ok, uint8_t *tx_fail, uint8_t *rx_ready);
//void RF24_startFastWrite(const void* buf, uint8_t len, const uint8_t multicast, uint8_t startTx = 1);
void RF24_startFastWrite(const void* buf, uint8_t len, const uint8_t multicast, uint8_t startTx);
uint8_t RF24_startWrite(const void* buf, uint8_t len, const uint8_t multicast);
void RF24_reUseTX();
uint8_t RF24_flush_tx(void);
uint8_t RF24_flush_rx(void);
uint8_t RF24_testCarrier(void);
uint8_t RF24_testRPD(void);
//uint8_t RF24_isValid();
void RF24_closeReadingPipe(uint8_t pipe);
uint8_t RF24_failureDetected;
void RF24_setAddressWidth(uint8_t a_width);
void RF24_setRetries(uint8_t delay, uint8_t count);
void RF24_setChannel(uint8_t channel);
uint8_t RF24_getChannel(void);
void RF24_setPayloadSize(uint8_t size);
uint8_t RF24_getPayloadSize(void);
uint8_t RF24_getDynamicPayloadSize(void);
void RF24_enableAckPayload(void);
void RF24_disableAckPayload(void);
void RF24_enableDynamicPayloads(void);
void RF24_disableDynamicPayloads(void);
void RF24_enableDynamicAck();
uint8_t RF24_isPVariant(void);
void RF24_setAutoAck(uint8_t enable);
//void RF24_setAutoAck(uint8_t pipe, uint8_t enable);
//void RF24_setPALevel(uint8_t level, uint8_t lnaEnable = 1);
void RF24_setPALevel(uint8_t level, uint8_t lnaEnable);
uint8_t RF24_getPALevel(void);
uint8_t RF24_getARC(void);
uint8_t RF24_setDataRate(rf24_datarate_e speed);
rf24_datarate_e RF24_getDataRate(void);
void RF24_setCRCLength(rf24_crclength_e length);
rf24_crclength_e RF24_getCRCLength(void);
void RF24_disableCRC(void);
void RF24_maskIRQ(uint8_t tx_ok, uint8_t tx_fail, uint8_t rx_ready);
uint32_t txDelay;
uint32_t csDelay;
void RF24_startConstCarrier(rf24_pa_dbm_e level, uint8_t channel);
void RF24_stopConstCarrier(void);
void RF24_openReadingPipe(uint8_t number, uint64_t address);
void RF24_openWritingPipe(uint64_t address);
uint8_t RF24_isAckPayloadAvailable(void);

void RF24_csn(uint8_t mode);
void RF24_ce(uint8_t level);
void RF24_read_registers(uint8_t reg, uint8_t* buf, uint8_t len);
uint8_t RF24_read_register(uint8_t reg);
void RF24_write_registers(uint8_t reg, const uint8_t* buf, uint8_t len);
//void RF24_write_register(uint8_t reg, uint8_t value, uint8_t is_cmd_only = 0);
void RF24_write_register(uint8_t reg, uint8_t value, uint8_t is_cmd_only);
void RF24_write_payload(const void* buf, uint8_t len, const uint8_t writeType);
void RF24_read_payload(void* buf, uint8_t len);
uint8_t RF24_get_status(void);
void RF24_toggle_features(void);

void RF24_errNotify(void);

#endif
