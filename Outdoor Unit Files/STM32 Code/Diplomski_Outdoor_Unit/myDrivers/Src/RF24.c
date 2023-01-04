/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "nRF24L01.h"
#include "RF24_config.h"
#include "RF24.h"

//uint8_t RF24_isValid()
//{
//  return ce_pin != 0xff && csn_pin != 0xff;
//}
/****************************************************************************/

void RF24_csn(uint8_t mode)
{
    HAL_GPIO_WritePin(csn_port, csn_pin, mode);
    HAL_Delay(1);
}

/****************************************************************************/

void RF24_ce(uint8_t level)
{
    HAL_GPIO_WritePin(ce_port, ce_pin, level);
    HAL_Delay(1);
}

/****************************************************************************/

void RF24_beginTransaction()
{
    //_SPI->beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
    RF24_csn(0);
}

/****************************************************************************/

void RF24_endTransaction()
{
    //_SPI->endTransaction();
    RF24_csn(1);
}

/****************************************************************************/

void RF24_read_registers(uint8_t reg, uint8_t* buf, uint8_t len)
{
    uint8_t _data = R_REGISTER | reg;
    RF24_beginTransaction();
    //status = _SPI->transfer(R_REGISTER | reg);
    HAL_SPI_TransmitReceive(&hspi1, &_data, &status, sizeof(uint8_t), 1000);
    //while (len--) {
    //    *buf++ = _SPI->transfer(0xFF);
    //}
    HAL_SPI_Transmit(&hspi1, buf, len, 1000);
    RF24_endTransaction();
}

/****************************************************************************/

uint8_t RF24_read_register(uint8_t reg)
{
    uint8_t result;
    uint8_t _dummyReg = 0xff;
    uint8_t _data = R_REGISTER | reg;
    RF24_beginTransaction();
    //status = _SPI->transfer(R_REGISTER | reg);
    //result = _SPI->transfer(0xff);

    HAL_SPI_TransmitReceive(&hspi1, &_data, &status, sizeof(uint8_t), 1000);
    HAL_SPI_TransmitReceive(&hspi1, &_dummyReg, &result, sizeof(uint8_t), 1000);
    RF24_endTransaction();

    return result;
}

/****************************************************************************/

void RF24_write_registers(uint8_t reg, const uint8_t* buf, uint8_t len)
{
    uint8_t _data = W_REGISTER | reg;
    RF24_beginTransaction();
    //status = _SPI->transfer(W_REGISTER | reg);
    HAL_SPI_TransmitReceive(&hspi1, &_data, &status, sizeof(uint8_t), 1000);
    //while (len--) {
    //    _SPI->transfer(*buf++);
    //}
    HAL_SPI_Transmit(&hspi1, (uint8_t*)buf, len, 1000);
    RF24_endTransaction();
}

/****************************************************************************/

void RF24_write_register(uint8_t reg, uint8_t value, uint8_t is_cmd_only)
{
    uint8_t _data = W_REGISTER | reg;
    uint8_t _dummyReg;
    if (is_cmd_only) {
        RF24_beginTransaction();
    //    status = _SPI->transfer(W_REGISTER | reg);
        HAL_SPI_TransmitReceive(&hspi1, &_data, &status, sizeof(uint8_t), 1000);
        RF24_endTransaction();
    }
    else {
        RF24_beginTransaction();
    //    status = _SPI->transfer(W_REGISTER | reg);
    //    _SPI->transfer(value);
        HAL_SPI_TransmitReceive(&hspi1, &_data, &status, sizeof(uint8_t), 1000);
        HAL_SPI_TransmitReceive(&hspi1, &value, &_dummyReg, sizeof(uint8_t), 1000);
        RF24_endTransaction();
    }
}

/****************************************************************************/

void RF24_write_payload(const void* buf, uint8_t data_len, const uint8_t writeType)
{
    const uint8_t* current = (const uint8_t*)(buf);
    uint8_t _dummyReg[32];
    memset(_dummyReg, 0, 32);

    uint8_t blank_len = !data_len ? 1 : 0;
    if (!dynamic_payloads_enabled) {
        data_len = rf24_min(data_len, payload_size);
        blank_len = payload_size - data_len;
    }
    else {
        data_len = rf24_min(data_len, 32);
    }

    RF24_beginTransaction();
    //status = _SPI->transfer(writeType);
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&writeType, &status, sizeof(uint8_t), 1000);
    //while (data_len--) {
    //    _SPI->transfer(*current++);
    //}
    HAL_SPI_Transmit(&hspi1, (uint8_t*)current, data_len, 1000);
    //while (blank_len--) {
    //    _SPI->transfer(0);
    //}
    HAL_SPI_Transmit(&hspi1, _dummyReg, blank_len, 1000);
    RF24_endTransaction();
}

/****************************************************************************/

void RF24_read_payload(void* buf, uint8_t data_len)
{
    uint8_t* current = (uint8_t*)(buf);
    uint8_t _dummyReg[32];
    uint8_t _reg;

    uint8_t blank_len = 0;
    if (!dynamic_payloads_enabled) {
        data_len = rf24_min(data_len, payload_size);
        blank_len = payload_size - data_len;
    }
    else {
        data_len = rf24_min(data_len, 32);
    }

    RF24_beginTransaction();
    //status = _SPI->transfer(R_RX_PAYLOAD);
    _reg = R_RX_PAYLOAD;
    HAL_SPI_TransmitReceive(&hspi1, &_reg, &status, sizeof(uint8_t), 1000);
    //while (data_len--) {
    //    *current++ = _SPI->transfer(0xFF);
    //}
    HAL_SPI_Receive(&hspi1, current, data_len, 1000);
    //while (blank_len--) {
    //    _SPI->transfer(0xff);
    //}
    HAL_SPI_Receive(&hspi1, _dummyReg, blank_len, 1000);
    RF24_endTransaction();
}

/****************************************************************************/

uint8_t RF24_flush_rx(void)
{
    RF24_write_register(FLUSH_RX, RF24_NOP, 1);
    return status;
}

/****************************************************************************/

uint8_t RF24_flush_tx(void)
{
    RF24_write_register(FLUSH_TX, RF24_NOP, 1);
    return status;
}

/****************************************************************************/

uint8_t RF24_get_status(void)
{
    RF24_write_register(RF24_NOP, RF24_NOP, 1);
    return status;
}

/****************************************************************************/

void RF24_init(GPIO_TypeDef* _ce_port, uint16_t _ce_pin, GPIO_TypeDef* _cs_port, uint16_t _cs_pin)
{
    ce_port = _ce_port;
    ce_pin = _ce_pin;
    csn_port = _cs_port;
    csn_pin = _cs_pin;
    payload_size = 32;
    dynamic_payloads_enabled = 1;
    addr_width = 5;
    _is_p_variant = 0;
    csDelay = 5;
    pipe0_reading_address[0] = 0;
}

/****************************************************************************/

void RF24_setChannel(uint8_t channel)
{
    const uint8_t max_channel = 125;
    RF24_write_register(RF_CH, rf24_min(channel, max_channel), 0);
}

uint8_t RF24_getChannel()
{

    return RF24_read_register(RF_CH);
}

/****************************************************************************/

void RF24_setPayloadSize(uint8_t size)
{
    // payload size must be in range [1, 32]
    payload_size = rf24_max(1, rf24_min(32, size));

    // write static payload size setting for all pipes
    for (uint8_t i = 0; i < 6; ++i)
        RF24_write_register(RX_PW_P0 + i, payload_size, 0);
}

/****************************************************************************/

uint8_t RF24_getPayloadSize(void)
{
    return payload_size;
}

/****************************************************************************/

uint8_t RF24_begin()
{
    RF24_ce(0);
    RF24_csn(1);

    // Must allow the radio time to settle else configuration bits will not necessarily stick.
    // This is actually only required following power up but some settling time also appears to
    // be required after resets too. For full coverage, we'll always assume the worst.
    // Enabling 16b CRC is by far the most obvious case if the wrong timing is used - or skipped.
    // Technically we require 4.5ms + 14us as a worst case. We'll just call it 5ms for good measure.
    // WARNING: Delay is based on P-variant whereby non-P *may* require different timing.
    HAL_Delay(5);

    // Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
    // WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
    // sizes must never be used. See datasheet for a more complete explanation.
    RF24_setRetries(5, 15);

    // Then set the data rate to the slowest (and most reliable) speed supported by all
    // hardware.
    RF24_setDataRate(RF24_1MBPS);

    // detect if is a plus variant & use old toggle features command accordingly
    uint8_t before_toggle = RF24_read_register(FEATURE);
    RF24_toggle_features();
    uint8_t after_toggle = RF24_read_register(FEATURE);
    _is_p_variant = before_toggle == after_toggle;
    if (after_toggle){
        if (_is_p_variant){
            // module did not experience power-on-reset (#401)
            RF24_toggle_features();
        }
        // allow use of multicast parameter and dynamic payloads by default
        RF24_write_register(FEATURE, 0, 0);
    }
    ack_payloads_enabled = 0;     // ack payloads disabled by default
    RF24_write_register(DYNPD, 0, 0);         // disable dynamic payloads by default (for all pipes)
    dynamic_payloads_enabled = 0;
    RF24_write_register(EN_AA, 0x3F, 0);      // enable auto-ack on all pipes
    RF24_write_register(EN_RXADDR, 0, 0);     // close all RX pipes
    RF24_setPayloadSize(32);               // set static payload size to 32 (max) bytes by default
    RF24_setAddressWidth(5);               // set default address length to (max) 5 bytes

    // Set up default configuration.  Callers can always change it later.
    // This channel should be universally safe and not bleed over into adjacent
    // spectrum.
    RF24_setChannel(76);

    // Reset current status
    // Notice reset and flush is the last thing we do
    RF24_write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT), 0);


    // Flush buffers
    RF24_flush_rx();
    RF24_flush_tx();

    // Clear CONFIG register:
    //      Reflect all IRQ events on IRQ pin
    //      Enable PTX
    //      Power Up
    //      16-bit CRC (CRC required by auto-ack)
    // Do not write CE high so radio will remain in standby I mode
    // PTX should use only 22uA of power
    RF24_write_register(NRF_CONFIG, (_BV(EN_CRC) | _BV(CRCO)), 0);
    config_reg = RF24_read_register(NRF_CONFIG);

    RF24_powerUp();

    // if config is not set correctly then there was a bad response from module
    return config_reg == (_BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP)) ? 1 : 0;
}

/****************************************************************************/

uint8_t RF24_isChipConnected()
{
    uint8_t setup = RF24_read_register(SETUP_AW);
    if (setup >= 1 && setup <= 3) {
        return 1;
    }

    return 0;
}

/****************************************************************************/

void RF24_startListening(void)
{
    config_reg |= _BV(PRIM_RX);
    RF24_write_register(NRF_CONFIG, config_reg, 0);
    RF24_write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT), 0);
    RF24_ce(1);

    // Restore the pipe0 address, if exists
    if (pipe0_reading_address[0] > 0) {
        RF24_write_registers(RX_ADDR_P0, pipe0_reading_address, addr_width);
    } else {
        RF24_closeReadingPipe(0);
    }
}

/****************************************************************************/
static const uint8_t child_pipe_enable[] = {ERX_P0, ERX_P1, ERX_P2,
                                                    ERX_P3, ERX_P4, ERX_P5};

void RF24_stopListening(void)
{
    RF24_ce(0);

    //delayMicroseconds(100);
    //delayMicroseconds(txDelay);
    HAL_Delay(1);
    if (ack_payloads_enabled){
        RF24_flush_tx();
    }

    config_reg &= ~_BV(PRIM_RX);
    RF24_write_register(NRF_CONFIG, config_reg, 0);

    RF24_write_register(EN_RXADDR, RF24_read_register(EN_RXADDR) | _BV(child_pipe_enable[0]), 0); // Enable RX on pipe0
}

/****************************************************************************/

void RF24_powerDown(void)
{
    RF24_ce(0); // Guarantee CE is low on powerDown
    config_reg &= ~_BV(PWR_UP);
    RF24_write_register(NRF_CONFIG, config_reg, 0);
}

/****************************************************************************/

//Power up now. Radio will not power down unless instructed by MCU for config changes etc.
void RF24_powerUp(void)
{
    // if not powered up then power up and wait for the radio to initialize
    if (!(config_reg & _BV(PWR_UP))) {
        config_reg |= _BV(PWR_UP);
        RF24_write_register(NRF_CONFIG, config_reg, 0);

        // For nRF24L01+ to go from power down mode to TX or RX mode it must first pass through stand-by mode.
        // There must be a delay of Tpd2stby (see Table 16.) after the nRF24L01+ leaves power down mode before
        // the CEis set high. - Tpd2stby can be up to 5ms per the 1.0 datasheet
        //delayMicroseconds(RF24_POWERUP_DELAY);
        HAL_Delay(1);
    }
}

/******************************************************************/
void RF24_errNotify()
{

}
/******************************************************************/

//Similar to the previous write, clears the interrupt flags
uint8_t RF24_write(const void* buf, uint8_t len, const uint8_t multicast)
{
    //Start Writing
    RF24_startFastWrite(buf, len, multicast, 1);

    //Wait until complete or failed
    #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
    uint32_t timer = HAL_GetTick();
    #endif // defined(FAILURE_HANDLING) || defined(RF24_LINUX)

    while (!(RF24_get_status() & (_BV(TX_DS) | _BV(MAX_RT)))) {
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (HAL_GetTick() - timer > 95) {
            RF24_errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #else
            HAL_Delay(100);
            #endif
        }
        #endif
    }

    RF24_ce(0);

    RF24_write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT), 0);

    //Max retries exceeded
    if (status & _BV(MAX_RT)) {
        RF24_flush_tx(); // Only going to be 1 packet in the FIFO at a time using this method, so just flush
        return 0;
    }
    //TX OK 1 or 0
    return 1;
}

//uint8_t RF24_write(const void* buf, uint8_t len)
//{
//    return RF24_write(buf, len, 0);
//}
/****************************************************************************/

//For general use, the interrupt flags are not important to clear
uint8_t RF24_writeBlocking(const void* buf, uint8_t len, uint32_t timeout)
{
    //Block until the FIFO is NOT full.
    //Keep track of the MAX retries and set auto-retry if seeing failures
    //This way the FIFO will fill up and allow blocking until packets go through
    //The radio will auto-clear everything in the FIFO as long as CE remains high

    uint32_t timer = HAL_GetTick();                               // Get the time that the payload transmission started

    while ((RF24_get_status() & (_BV(TX_FULL)))) {                // Blocking only if FIFO is full. This will loop and block until TX is successful or timeout

        if (status & _BV(MAX_RT)) {                          // If MAX Retries have been reached
            RF24_reUseTX();                                       // Set re-transmit and clear the MAX_RT interrupt flag
            if (HAL_GetTick() - timer > timeout) {
                return 0;                                    // If this payload has exceeded the user-defined timeout, exit and return 0
            }
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (HAL_GetTick() - timer > (timeout + 95)) {
            RF24_errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif
        }
        #endif

    }

    //Start Writing
    RF24_startFastWrite(buf, len, 0, 1);                             // Write the payload if a buffer is clear

    return 1;                                                // Return 1 to indicate successful transmission
}

/****************************************************************************/

void RF24_reUseTX()
{
    RF24_write_register(NRF_STATUS, _BV(MAX_RT), 0);  //Clear max retry flag
    RF24_write_register(REUSE_TX_PL, RF24_NOP, 1);
    RF24_ce(0);                                  //Re-Transfer packet
    RF24_ce(1);
}

/****************************************************************************/

uint8_t RF24_writeFast(const void* buf, uint8_t len, const uint8_t multicast)
{
    //Block until the FIFO is NOT full.
    //Keep track of the MAX retries and set auto-retry if seeing failures
    //Return 0 so the user can control the retrys and set a timer or failure counter if required
    //The radio will auto-clear everything in the FIFO as long as CE remains high

    #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
    uint32_t timer = HAL_GetTick();
    #endif

    //Blocking only if FIFO is full. This will loop and block until TX is successful or fail
    while ((RF24_get_status() & (_BV(TX_FULL)))) {
        if (status & _BV(MAX_RT)) {
            return 0;                                        //Return 0. The previous payload has not been retransmitted
            // From the user perspective, if you get a 0, just keep trying to send the same payload
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (HAL_GetTick() - timer > 95) {
            RF24_errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif // defined(FAILURE_HANDLING)
        }
        #endif
    }
    RF24_startFastWrite(buf, len, multicast, 1);                     // Start Writing

    return 1;
}

//uint8_t RF24_writeFast(const void* buf, uint8_t len)
//{
//    return RF24_writeFast(buf, len, 0);
//}

/****************************************************************************/

//Per the documentation, we want to set PTX Mode when not listening. Then all we do is write data and set CE high
//In this mode, if we can keep the FIFO buffers loaded, packets will transmit immediately (no 130us delay)
//Otherwise we enter Standby-II mode, which is still faster than standby mode
//Also, we remove the need to keep writing the config register over and over and delaying for 150 us each time if sending a stream of data

void RF24_startFastWrite(const void* buf, uint8_t len, const uint8_t multicast, uint8_t startTx)
{ //TMRh20

    RF24_write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);
    if (startTx) {
        RF24_ce(1);
    }
}

/****************************************************************************/

//Added the original startWrite back in so users can still use interrupts, ack payloads, etc
//Allows the library to pass all tests
uint8_t RF24_startWrite(const void* buf, uint8_t len, const uint8_t multicast)
{

    // Send the payload
    RF24_write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);
    RF24_ce(1);
    //delayMicroseconds(10);
    HAL_Delay(1);
    RF24_ce(0);
    return !(status & _BV(TX_FULL));
}

/****************************************************************************/

uint8_t RF24_rxFifoFull()
{
    return RF24_read_register(FIFO_STATUS) & _BV(RX_FULL);
}

/****************************************************************************/

uint8_t RF24_txStandBy()
{

    #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
    uint32_t timeout = HAL_GetTick();
    #endif
    while (!(RF24_read_register(FIFO_STATUS) & _BV(TX_EMPTY))) {
        if (status & _BV(MAX_RT)) {
            RF24_write_register(NRF_STATUS, _BV(MAX_RT), 0);
            RF24_ce(0);
            RF24_flush_tx();    //Non blocking, flush the data
            return 0;
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (HAL_GetTick() - timeout > 95) {
            RF24_errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif
        }
        #endif
    }

    RF24_ce(0);               //Set STANDBY-I mode
    return 1;
}

/****************************************************************************/

uint8_t RF24_txStandByT(uint32_t timeout, uint8_t startTx)
{

    if (startTx) {
        RF24_stopListening();
        RF24_ce(1);
    }
    uint32_t start = HAL_GetTick();

    while (!(RF24_read_register(FIFO_STATUS) & _BV(TX_EMPTY))) {
        if (status & _BV(MAX_RT)) {
            RF24_write_register(NRF_STATUS, _BV(MAX_RT), 0);
            RF24_ce(0); // Set re-transmit
            RF24_ce(1);
            if (HAL_GetTick() - start >= timeout) {
                RF24_ce(0);
                RF24_flush_tx();
                return 0;
            }
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (HAL_GetTick() - start > (timeout + 95)) {
            RF24_errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif
        }
        #endif
    }

    RF24_ce(0);  //Set STANDBY-I mode
    return 1;

}

/****************************************************************************/

void RF24_maskIRQ(uint8_t tx, uint8_t fail, uint8_t rx)
{
    /* clear the interrupt flags */
    config_reg &= ~(1 << MASK_MAX_RT | 1 << MASK_TX_DS | 1 << MASK_RX_DR);
    /* set the specified interrupt flags */
    config_reg |= fail << MASK_MAX_RT | tx << MASK_TX_DS | rx << MASK_RX_DR;
    RF24_write_register(NRF_CONFIG, config_reg, 0);
}

/****************************************************************************/

uint8_t RF24_getDynamicPayloadSize(void)
{
    uint8_t result = RF24_read_register(R_RX_PL_WID);

    if (result > 32) {
        RF24_flush_rx();
        HAL_Delay(2);
        return 0;
    }
    return result;
}

/****************************************************************************/

//uint8_t RF24_available(void)
//{
//    return available(NULL);
//}

/****************************************************************************/

uint8_t RF24_available(uint8_t* pipe_num)
{
    // get implied RX FIFO empty flag from status byte
    uint8_t pipe = (RF24_get_status() >> RX_P_NO) & 0x07;
    if (pipe > 5)
        return 0;

    // If the caller wants the pipe number, include that
    if (pipe_num)
        *pipe_num = pipe;

    return 1;
}

/****************************************************************************/

void RF24_read(void* buf, uint8_t len)
{

    // Fetch the payload
    RF24_read_payload(buf, len);

    //Clear the only applicable interrupt flags
    RF24_write_register(NRF_STATUS, _BV(RX_DR), 0);

}

/****************************************************************************/

void RF24_whatHappened(uint8_t *tx_ok, uint8_t *tx_fail, uint8_t *rx_ready)
{
    // Read the status & reset the status in one easy call
    // Or is that such a good idea?
    RF24_write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT), 0);

    // Report to the user what happened
    *tx_ok = status & _BV(TX_DS);
    *tx_fail = status & _BV(MAX_RT);
    *rx_ready = status & _BV(RX_DR);
}

/****************************************************************************/

void RF24_openWritingPipe(uint64_t value)
{
    // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
    // expects it LSB first too, so we're good.

    RF24_write_registers(RX_ADDR_P0, (uint8_t*)(&value), addr_width);
    RF24_write_registers(TX_ADDR, (uint8_t*)(&value), addr_width);
}

/****************************************************************************/
//void RF24_openWritingPipe(const uint8_t* address)
//{
    // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
    // expects it LSB first too, so we're good.
//    RF24_write_registers(RX_ADDR_P0, address, addr_width);
//    RF24_write_registers(TX_ADDR, address, addr_width);
//}

/****************************************************************************/
const uint8_t child_pipe[] = {RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2,
                                             RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};

void RF24_openReadingPipe(uint8_t child, uint64_t address)
{
    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (child == 0) {
        memcpy(pipe0_reading_address, &address, addr_width);
    }

    if (child <= 5) {
        // For pipes 2-5, only write the LSB
        if (child < 2) {
            RF24_write_registers(child_pipe[child], (const uint8_t*)(&address), addr_width);
        } else {
            RF24_write_registers(child_pipe[child], (const uint8_t*)(&address), 1);
        }

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        RF24_write_register(EN_RXADDR, RF24_read_register(EN_RXADDR) | _BV(child_pipe_enable[child]), 0);
    }
}

/****************************************************************************/
void RF24_setAddressWidth(uint8_t a_width)
{

    if (a_width -= 2) {
        RF24_write_register(SETUP_AW, a_width % 4, 0);
        addr_width = (a_width % 4) + 2;
    } else {
        RF24_write_register(SETUP_AW, 0, 0);
        addr_width = 2;
    }

}

/****************************************************************************/

void RF24openReadingPipe(uint8_t child, const uint8_t* address)
{
    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (child == 0) {
        memcpy(pipe0_reading_address, address, addr_width);
    }
    if (child <= 5) {
        // For pipes 2-5, only write the LSB
        if (child < 2) {
            RF24_write_registers(child_pipe[child], address, addr_width);
        } else {
            RF24_write_registers(child_pipe[child], address, 1);
        }

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        RF24_write_register(EN_RXADDR, RF24_read_register(EN_RXADDR) | _BV(child_pipe_enable[child]), 0);

    }
}

/****************************************************************************/

void RF24_closeReadingPipe(uint8_t pipe)
{
    RF24_write_register(EN_RXADDR, RF24_read_register(EN_RXADDR) & ~_BV(child_pipe_enable[pipe]), 0);
}

/****************************************************************************/

void RF24_toggle_features(void)
{
    uint8_t _data = ACTIVATE;
    RF24_beginTransaction();
    //status = _SPI->transfer(ACTIVATE);
    //_SPI->transfer(0x73);
    HAL_SPI_TransmitReceive(&hspi1, &_data, &status, sizeof(uint8_t), 1000);
    _data = 0x73;
    HAL_SPI_Transmit(&hspi1, &_data, sizeof(uint8_t), 1000);
    RF24_endTransaction();
}

/****************************************************************************/

void RF24_enableDynamicPayloads(void)
{
    // Enable dynamic payload throughout the system

    //toggle_features();
    RF24_write_register(FEATURE, RF24_read_register(FEATURE) | _BV(EN_DPL), 0);

    // Enable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    RF24_write_register(DYNPD, RF24_read_register(DYNPD) | _BV(DPL_P5) | _BV(DPL_P4) | _BV(DPL_P3) | _BV(DPL_P2) | _BV(DPL_P1) | _BV(DPL_P0), 0);

    dynamic_payloads_enabled = 1;
}

/****************************************************************************/
void RF24_disableDynamicPayloads(void)
{
    // Disables dynamic payload throughout the system.  Also disables Ack Payloads

    //toggle_features();
    RF24_write_register(FEATURE, 0, 0);

    // Disable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    RF24_write_register(DYNPD, 0, 0);

    dynamic_payloads_enabled = 0;
    ack_payloads_enabled = 0;
}

/****************************************************************************/

void RF24_enableAckPayload(void)
{
    // enable ack payloads and dynamic payload features

    if (!ack_payloads_enabled){
        RF24_write_register(FEATURE, RF24_read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL), 0);

        // Enable dynamic payload on pipes 0
        RF24_write_register(DYNPD, RF24_read_register(DYNPD) | _BV(DPL_P1) | _BV(DPL_P0), 0);
        dynamic_payloads_enabled = 1;
        ack_payloads_enabled = 1;
    }
}

/****************************************************************************/

void RF24_disableAckPayload(void)
{
    // disable ack payloads (leave dynamic payload features as is)
    if (ack_payloads_enabled){
        RF24_write_register(FEATURE, RF24_read_register(FEATURE) | ~_BV(EN_ACK_PAY), 0);

        ack_payloads_enabled = 0;
    }
}

/****************************************************************************/

void RF24_enableDynamicAck(void)
{
    //
    // enable dynamic ack features
    //
    //toggle_features();
    RF24_write_register(FEATURE, RF24_read_register(FEATURE) | _BV(EN_DYN_ACK), 0);
}

/****************************************************************************/

uint8_t RF24_writeAckPayload(uint8_t pipe, const void* buf, uint8_t len)
{
    if (ack_payloads_enabled){
        const uint8_t* current = (const uint8_t*)(buf);

        RF24_write_payload(current, len, W_ACK_PAYLOAD | (pipe & 0x07));
        return !(status & _BV(TX_FULL));
    }
    return 0;
}

/****************************************************************************/

uint8_t RF24_isAckPayloadAvailable(void)
{
    return RF24_available(NULL);
}

/****************************************************************************/

uint8_t RF24_isPVariant(void)
{
    return _is_p_variant;
}

/****************************************************************************/

void RF24_setAutoAck(uint8_t enable)
{
    if (enable){
        RF24_write_register(EN_AA, 0x3F, 0);
    }else{
        RF24_write_register(EN_AA, 0, 0);
        // accomodate ACK payloads feature
        if (ack_payloads_enabled){
            RF24_disableAckPayload();
        }
    }
}

/****************************************************************************/

//void RF24_setAutoAck(uint8_t pipe, bool enable)
//{
//    if (pipe < 6) {
//        uint8_t en_aa = RF24_read_register(EN_AA);
//        if (enable) {
//            en_aa |= _BV(pipe);
//        }else{
//            en_aa &= ~_BV(pipe);
//            if (ack_payloads_enabled && !pipe){
//                RF24_disableAckPayload();
//            }
//        }
//        RF24_write_register(EN_AA, en_aa, 0);
//    }
//}

/****************************************************************************/

uint8_t RF24_testCarrier(void)
{
    return (RF24_read_register(CD) & 1);
}

/****************************************************************************/

uint8_t RF24_testRPD(void)
{
    return (RF24_read_register(RPD) & 1);
}

/****************************************************************************/

void RF24_setPALevel(uint8_t level, uint8_t lnaEnable)
{

    uint8_t setup = RF24_read_register(RF_SETUP) & 0xF8;

    if (level > 3) {                            // If invalid level, go to max PA
        level = (RF24_PA_MAX << 1) + lnaEnable; // +1 to support the SI24R1 chip extra bit
    } else {
        level = (level << 1) + lnaEnable;       // Else set level as requested
    }

    RF24_write_register(RF_SETUP, setup |= level, 0);   // Write it to the chip
}

/****************************************************************************/

uint8_t RF24_getPALevel(void)
{

    return (RF24_read_register(RF_SETUP) & (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH))) >> 1;
}

/****************************************************************************/

uint8_t RF24_getARC(void)
{

    return RF24_read_register(OBSERVE_TX) & 0x0F;
}

/****************************************************************************/

uint8_t RF24_setDataRate(rf24_datarate_e speed)
{
    uint8_t result = 0;
    uint8_t setup = RF24_read_register(RF_SETUP);

    // HIGH and LOW '00' is 1Mbs - our default
    setup &= ~(_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));
    txDelay = 280;
    if (speed == RF24_250KBPS) {
        // Must set the RF_DR_LOW to 1; RF_DR_HIGH (used to be RF_DR) is already 0
        // Making it '10'.
        setup |= _BV(RF_DR_LOW);
        txDelay = 505;
    } else {
        // Set 2Mbs, RF_DR (RF_DR_HIGH) is set 1
        // Making it '01'
        if (speed == RF24_2MBPS) {
            setup |= _BV(RF_DR_HIGH);
            txDelay = 240;
        }
    }
    RF24_write_register(RF_SETUP, setup, 0);

    // Verify our result
    if (RF24_read_register(RF_SETUP) == setup) {
        result = 1;
    }
    return result;
}

/****************************************************************************/

rf24_datarate_e RF24_getDataRate(void)
{
    rf24_datarate_e result;
    uint8_t dr = RF24_read_register(RF_SETUP) & (_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

    // switch uses RAM (evil!)
    // Order matters in our case below
    if (dr == _BV(RF_DR_LOW)) {
        // '10' = 250KBPS
        result = RF24_250KBPS;
    } else if (dr == _BV(RF_DR_HIGH)) {
        // '01' = 2MBPS
        result = RF24_2MBPS;
    } else {
        // '00' = 1MBPS
        result = RF24_1MBPS;
    }
    return result;
}

/****************************************************************************/

void RF24_setCRCLength(rf24_crclength_e length)
{
    config_reg &= ~(_BV(CRCO) | _BV(EN_CRC));

    // switch uses RAM (evil!)
    if (length == RF24_CRC_DISABLED) {
        // Do nothing, we turned it off above.
    } else if (length == RF24_CRC_8) {
        config_reg |= _BV(EN_CRC);
    } else {
        config_reg |= _BV(EN_CRC);
        config_reg |= _BV(CRCO);
    }
    RF24_write_register(NRF_CONFIG, config_reg, 0);
}

/****************************************************************************/

rf24_crclength_e RF24_getCRCLength(void)
{
    rf24_crclength_e result = RF24_CRC_DISABLED;
    uint8_t AA = RF24_read_register(EN_AA);
    config_reg = RF24_read_register(NRF_CONFIG);

    if (config_reg & _BV(EN_CRC) || AA) {
        if (config_reg & _BV(CRCO)) {
            result = RF24_CRC_16;
        } else {
            result = RF24_CRC_8;
        }
    }

    return result;
}

/****************************************************************************/

void RF24_disableCRC(void)
{
    config_reg &= ~_BV(EN_CRC);
    RF24_write_register(NRF_CONFIG, config_reg, 0);
}

/****************************************************************************/
void RF24_setRetries(uint8_t delay, uint8_t count)
{
    RF24_write_register(SETUP_RETR, (delay & 0xf) << ARD | (count & 0xf) << ARC, 0);
}

/****************************************************************************/
void RF24_startConstCarrier(rf24_pa_dbm_e level, uint8_t channel)
{
    RF24_stopListening();
    RF24_write_register(RF_SETUP, RF24_read_register(RF_SETUP) | _BV(CONT_WAVE) | _BV(PLL_LOCK), 0);
    if (RF24_isPVariant()){
        RF24_setAutoAck(0);
        RF24_setRetries(0, 0);
        uint8_t dummy_buf[32];
        for (uint8_t i = 0; i < 32; ++i)
            dummy_buf[i] = 0xFF;

        // use write_register() instead of openWritingPipe() to bypass
        // truncation of the address with the current RF24::addr_width value
        RF24_write_registers(TX_ADDR, (uint8_t*)(&dummy_buf), 5);
        RF24_flush_tx();  // so we can write to top level

        // use write_register() instead of write_payload() to bypass
        // truncation of the payload with the current RF24::payload_size value
        RF24_write_registers(W_TX_PAYLOAD, (const uint8_t*)(&dummy_buf), 32);

        RF24_disableCRC();
    }
    RF24_setPALevel(level, 1);
    RF24_setChannel(channel);
    RF24_ce(1);
    if (RF24_isPVariant()){
        HAL_Delay(1); // datasheet says 1 ms is ok in this instance
        RF24_ce(0);
        RF24_reUseTX();
    }
}

/****************************************************************************/
void RF24_stopConstCarrier()
{
    /*
     * A note from the datasheet:
     * Do not use REUSE_TX_PL together with CONT_WAVE=1. When both these
     * registers are set the chip does not react when setting CE low. If
     * however, both registers are set PWR_UP = 0 will turn TX mode off.
     */
    RF24_powerDown();  // per datasheet recommendation (just to be safe)
    RF24_write_register(RF_SETUP, RF24_read_register(RF_SETUP) & ~_BV(CONT_WAVE) & ~_BV(PLL_LOCK), 0);
    RF24_ce(0);
}
