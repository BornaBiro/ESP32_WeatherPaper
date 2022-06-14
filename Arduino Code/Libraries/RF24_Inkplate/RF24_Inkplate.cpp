/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "nRF24L01.h"
#include "RF24_config.h"
#include "RF24_Inkplate.h"


/****************************************************************************/

void RF24_Inkplate::csn(bool mode)
{
    //_SPI->setBitOrder(MSBFIRST);
    //_SPI->setDataMode(SPI_MODE0);
    //_SPI.setClockDivider(SPI_CLOCK_DIV2);

    _ink->digitalWriteInternal(MCP23017_INT_ADDR, _ink->mcpRegsInt, 14, mode);
    delayMicroseconds(csDelay);
}

/****************************************************************************/

void RF24_Inkplate::ce(bool level)
{
    _ink->digitalWriteInternal(MCP23017_INT_ADDR, _ink->mcpRegsInt, 15, level);
}

/****************************************************************************/

inline void RF24_Inkplate::beginTransaction()
{
    _SPI->beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
    csn(LOW);
}

/****************************************************************************/

inline void RF24_Inkplate::endTransaction()
{
    _SPI->endTransaction();
    csn(HIGH);
}

/****************************************************************************/

void RF24_Inkplate::read_register(uint8_t reg, uint8_t* buf, uint8_t len)
{
    beginTransaction();
    status = _SPI->transfer(R_REGISTER | reg);
    while (len--) {
        *buf++ = _SPI->transfer(0xFF);
    }
    endTransaction();
}

/****************************************************************************/

uint8_t RF24_Inkplate::read_register(uint8_t reg)
{
    uint8_t result;

    beginTransaction();
    status = _SPI->transfer(R_REGISTER | reg);
    result = _SPI->transfer(0xff);
    endTransaction();

    return result;
}

/****************************************************************************/

void RF24_Inkplate::write_register(uint8_t reg, const uint8_t* buf, uint8_t len)
{
    beginTransaction();
    status = _SPI->transfer(W_REGISTER | reg);
    while (len--) {
        _SPI->transfer(*buf++);
    }
    endTransaction();
}

/****************************************************************************/

void RF24_Inkplate::write_register(uint8_t reg, uint8_t value, bool is_cmd_only)
{
    if (is_cmd_only) {
        beginTransaction();
        status = _SPI->transfer(W_REGISTER | reg);
        endTransaction();
    }
    else {
        beginTransaction();
        status = _SPI->transfer(W_REGISTER | reg);
        _SPI->transfer(value);
        endTransaction();
    }
}

/****************************************************************************/

void RF24_Inkplate::write_payload(const void* buf, uint8_t data_len, const uint8_t writeType)
{
    const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);

    uint8_t blank_len = !data_len ? 1 : 0;
    if (!dynamic_payloads_enabled) {
        data_len = rf24_min(data_len, payload_size);
        blank_len = payload_size - data_len;
    }
    else {
        data_len = rf24_min(data_len, 32);
    }

    beginTransaction();
    status = _SPI->transfer(writeType);
    while (data_len--) {
        _SPI->transfer(*current++);
    }
    while (blank_len--) {
        _SPI->transfer(0);
    }
    endTransaction();
}

/****************************************************************************/

void RF24_Inkplate::read_payload(void* buf, uint8_t data_len)
{
    uint8_t* current = reinterpret_cast<uint8_t*>(buf);

    uint8_t blank_len = 0;
    if (!dynamic_payloads_enabled) {
        data_len = rf24_min(data_len, payload_size);
        blank_len = payload_size - data_len;
    }
    else {
        data_len = rf24_min(data_len, 32);
    }

    beginTransaction();
    status = _SPI->transfer(R_RX_PAYLOAD);
    while (data_len--) {
        *current++ = _SPI->transfer(0xFF);
    }
    while (blank_len--) {
        _SPI->transfer(0xff);
    }
    endTransaction();
}

/****************************************************************************/

uint8_t RF24_Inkplate::flush_rx(void)
{
    write_register(FLUSH_RX, RF24_NOP, true);
    return status;
}

/****************************************************************************/

uint8_t RF24_Inkplate::flush_tx(void)
{
    write_register(FLUSH_TX, RF24_NOP, true);
    return status;
}

/****************************************************************************/

uint8_t RF24_Inkplate::get_status(void)
{
    write_register(RF24_NOP, RF24_NOP, true);
    return status;
}

/****************************************************************************/

RF24_Inkplate::RF24_Inkplate(uint16_t _cepin, uint16_t _cspin, uint32_t _spi_speed)
        :ce_pin(_cepin), csn_pin(_cspin),spi_speed(_spi_speed), payload_size(32), dynamic_payloads_enabled(true), addr_width(5), _is_p_variant(false),
         csDelay(5)
{
    pipe0_reading_address[0] = 0;
    if(spi_speed <= 35000){ //Handle old BCM2835 speed constants, default to RF24_SPI_SPEED
      spi_speed = RF24_SPI_SPEED;
    }
}

/****************************************************************************/

void RF24_Inkplate::setChannel(uint8_t channel)
{
    const uint8_t max_channel = 125;
    write_register(RF_CH, rf24_min(channel, max_channel));
}

uint8_t RF24_Inkplate::getChannel()
{

    return read_register(RF_CH);
}

/****************************************************************************/

void RF24_Inkplate::setPayloadSize(uint8_t size)
{
    // payload size must be in range [1, 32]
    payload_size = rf24_max(1, rf24_min(32, size));

    // write static payload size setting for all pipes
    for (uint8_t i = 0; i < 6; ++i)
        write_register(RX_PW_P0 + i, payload_size);
}

/****************************************************************************/

uint8_t RF24_Inkplate::getPayloadSize(void)
{
    return payload_size;
}

/****************************************************************************/

bool RF24_Inkplate::begin(SPIClass *s, Inkplate *i)
{
    _SPI = s;
    _ink = i;
    // Initialize pins
    if (ce_pin != csn_pin) {
      _ink->pinModeInternal(MCP23017_INT_ADDR, _ink->mcpRegsInt, ce_pin, OUTPUT);
      _ink->pinModeInternal(MCP23017_INT_ADDR, _ink->mcpRegsInt, csn_pin, OUTPUT);
    }
    _SPI->begin(14, 12, 13, 15);
    ce(LOW);
    csn(HIGH);

    // Must allow the radio time to settle else configuration bits will not necessarily stick.
    // This is actually only required following power up but some settling time also appears to
    // be required after resets too. For full coverage, we'll always assume the worst.
    // Enabling 16b CRC is by far the most obvious case if the wrong timing is used - or skipped.
    // Technically we require 4.5ms + 14us as a worst case. We'll just call it 5ms for good measure.
    // WARNING: Delay is based on P-variant whereby non-P *may* require different timing.
    delay(5);

    // Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
    // WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
    // sizes must never be used. See datasheet for a more complete explanation.
    setRetries(5, 15);
    
    // Then set the data rate to the slowest (and most reliable) speed supported by all
    // hardware.
    setDataRate(RF24_1MBPS);
  
    // detect if is a plus variant & use old toggle features command accordingly
    uint8_t before_toggle = read_register(FEATURE);
    toggle_features();
    uint8_t after_toggle = read_register(FEATURE);
    _is_p_variant = before_toggle == after_toggle;
    if (after_toggle){
        if (_is_p_variant){
            // module did not experience power-on-reset (#401)
            toggle_features();
        }
        // allow use of multicast parameter and dynamic payloads by default
        write_register(FEATURE, 0);
    }
    ack_payloads_enabled = false;     // ack payloads disabled by default
    write_register(DYNPD, 0);         // disable dynamic payloads by default (for all pipes)
    dynamic_payloads_enabled = false;
    write_register(EN_AA, 0x3F);      // enable auto-ack on all pipes
    write_register(EN_RXADDR, 0);     // close all RX pipes
    setPayloadSize(32);               // set static payload size to 32 (max) bytes by default
    setAddressWidth(5);               // set default address length to (max) 5 bytes

    // Set up default configuration.  Callers can always change it later.
    // This channel should be universally safe and not bleed over into adjacent
    // spectrum.
    setChannel(76);

    // Reset current status
    // Notice reset and flush is the last thing we do
    write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));


    // Flush buffers
    flush_rx();
    flush_tx();

    // Clear CONFIG register:
    //      Reflect all IRQ events on IRQ pin
    //      Enable PTX
    //      Power Up
    //      16-bit CRC (CRC required by auto-ack)
    // Do not write CE high so radio will remain in standby I mode
    // PTX should use only 22uA of power
    write_register(NRF_CONFIG, (_BV(EN_CRC) | _BV(CRCO)) );
    config_reg = read_register(NRF_CONFIG);

    powerUp();

    // if config is not set correctly then there was a bad response from module
    return config_reg == (_BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP)) ? true : false;
}

/****************************************************************************/

bool RF24_Inkplate::isChipConnected()
{
    uint8_t setup = read_register(SETUP_AW);
    if (setup >= 1 && setup <= 3) {
        return true;
    }

    return false;
}

/****************************************************************************/

void RF24_Inkplate::startListening(void)
{
    config_reg |= _BV(PRIM_RX);
    write_register(NRF_CONFIG, config_reg);
    write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));
    ce(HIGH);

    // Restore the pipe0 address, if exists
    if (pipe0_reading_address[0] > 0) {
        write_register(RX_ADDR_P0, pipe0_reading_address, addr_width);
    } else {
        closeReadingPipe(0);
    }
}

/****************************************************************************/
static const uint8_t child_pipe_enable[] = {ERX_P0, ERX_P1, ERX_P2,
                                                    ERX_P3, ERX_P4, ERX_P5};

void RF24_Inkplate::stopListening(void)
{
    ce(LOW);

    //delayMicroseconds(100);
    delayMicroseconds(txDelay);
    if (ack_payloads_enabled){
        flush_tx();
    }

    config_reg &= ~_BV(PRIM_RX);
    write_register(NRF_CONFIG, config_reg);

    write_register(EN_RXADDR, read_register(EN_RXADDR) | _BV(child_pipe_enable[0])); // Enable RX on pipe0
}

/****************************************************************************/

void RF24_Inkplate::powerDown(void)
{
    ce(LOW); // Guarantee CE is low on powerDown
    config_reg &= ~_BV(PWR_UP);
    write_register(NRF_CONFIG,config_reg);
}

/****************************************************************************/

//Power up now. Radio will not power down unless instructed by MCU for config changes etc.
void RF24_Inkplate::powerUp(void)
{
    // if not powered up then power up and wait for the radio to initialize
    if (!(config_reg & _BV(PWR_UP))) {
        config_reg |= _BV(PWR_UP);
        write_register(NRF_CONFIG, config_reg);

        // For nRF24L01+ to go from power down mode to TX or RX mode it must first pass through stand-by mode.
        // There must be a delay of Tpd2stby (see Table 16.) after the nRF24L01+ leaves power down mode before
        // the CEis set high. - Tpd2stby can be up to 5ms per the 1.0 datasheet
        delayMicroseconds(RF24_POWERUP_DELAY);
    }
}

/******************************************************************/
#if defined(FAILURE_HANDLING) || defined(RF24_LINUX)

void RF24_Inkplate::errNotify()
{
    
}

#endif
/******************************************************************/

//Similar to the previous write, clears the interrupt flags
bool RF24_Inkplate::write(const void* buf, uint8_t len, const bool multicast)
{
    //Start Writing
    startFastWrite(buf, len, multicast);

    //Wait until complete or failed
    #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
    uint32_t timer = millis();
    #endif // defined(FAILURE_HANDLING) || defined(RF24_LINUX)

    while (!(get_status() & (_BV(TX_DS) | _BV(MAX_RT)))) {
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (millis() - timer > 95) {
            errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #else
            delay(100);
            #endif
        }
        #endif
    }

    ce(LOW);

    write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

    //Max retries exceeded
    if (status & _BV(MAX_RT)) {
        flush_tx(); // Only going to be 1 packet in the FIFO at a time using this method, so just flush
        return 0;
    }
    //TX OK 1 or 0
    return 1;
}

bool RF24_Inkplate::write(const void* buf, uint8_t len)
{
    return write(buf, len, 0);
}
/****************************************************************************/

//For general use, the interrupt flags are not important to clear
bool RF24_Inkplate::writeBlocking(const void* buf, uint8_t len, uint32_t timeout)
{
    //Block until the FIFO is NOT full.
    //Keep track of the MAX retries and set auto-retry if seeing failures
    //This way the FIFO will fill up and allow blocking until packets go through
    //The radio will auto-clear everything in the FIFO as long as CE remains high

    uint32_t timer = millis();                               // Get the time that the payload transmission started

    while ((get_status() & (_BV(TX_FULL)))) {                // Blocking only if FIFO is full. This will loop and block until TX is successful or timeout

        if (status & _BV(MAX_RT)) {                          // If MAX Retries have been reached
            reUseTX();                                       // Set re-transmit and clear the MAX_RT interrupt flag
            if (millis() - timer > timeout) {
                return 0;                                    // If this payload has exceeded the user-defined timeout, exit and return 0
            }
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (millis() - timer > (timeout + 95)) {
            errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif
        }
        #endif

    }

    //Start Writing
    startFastWrite(buf, len, 0);                             // Write the payload if a buffer is clear

    return 1;                                                // Return 1 to indicate successful transmission
}

/****************************************************************************/

void RF24_Inkplate::reUseTX()
{
    write_register(NRF_STATUS, _BV(MAX_RT));  //Clear max retry flag
    write_register(REUSE_TX_PL, RF24_NOP, true);
    ce(LOW);                                  //Re-Transfer packet
    ce(HIGH);
}

/****************************************************************************/

bool RF24_Inkplate::writeFast(const void* buf, uint8_t len, const bool multicast)
{
    //Block until the FIFO is NOT full.
    //Keep track of the MAX retries and set auto-retry if seeing failures
    //Return 0 so the user can control the retrys and set a timer or failure counter if required
    //The radio will auto-clear everything in the FIFO as long as CE remains high

    #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
    uint32_t timer = millis();
    #endif

    //Blocking only if FIFO is full. This will loop and block until TX is successful or fail
    while ((get_status() & (_BV(TX_FULL)))) {
        if (status & _BV(MAX_RT)) {
            return 0;                                        //Return 0. The previous payload has not been retransmitted
            // From the user perspective, if you get a 0, just keep trying to send the same payload
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (millis() - timer > 95) {
            errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif // defined(FAILURE_HANDLING)
        }
        #endif
    }
    startFastWrite(buf, len, multicast);                     // Start Writing

    return 1;
}

bool RF24_Inkplate::writeFast(const void* buf, uint8_t len)
{
    return writeFast(buf, len, 0);
}

/****************************************************************************/

//Per the documentation, we want to set PTX Mode when not listening. Then all we do is write data and set CE high
//In this mode, if we can keep the FIFO buffers loaded, packets will transmit immediately (no 130us delay)
//Otherwise we enter Standby-II mode, which is still faster than standby mode
//Also, we remove the need to keep writing the config register over and over and delaying for 150 us each time if sending a stream of data

void RF24_Inkplate::startFastWrite(const void* buf, uint8_t len, const bool multicast, bool startTx)
{ //TMRh20

    write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);
    if (startTx) {
        ce(HIGH);
    }
}

/****************************************************************************/

//Added the original startWrite back in so users can still use interrupts, ack payloads, etc
//Allows the library to pass all tests
bool RF24_Inkplate::startWrite(const void* buf, uint8_t len, const bool multicast)
{

    // Send the payload
    write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);
    ce(HIGH);
    delayMicroseconds(10);
    ce(LOW);
    return !(status & _BV(TX_FULL));
}

/****************************************************************************/

bool RF24_Inkplate::rxFifoFull()
{
    return read_register(FIFO_STATUS) & _BV(RX_FULL);
}

/****************************************************************************/

bool RF24_Inkplate::txStandBy()
{

    #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
    uint32_t timeout = millis();
    #endif
    while (!(read_register(FIFO_STATUS) & _BV(TX_EMPTY))) {
        if (status & _BV(MAX_RT)) {
            write_register(NRF_STATUS, _BV(MAX_RT));
            ce(LOW);
            flush_tx();    //Non blocking, flush the data
            return 0;
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (millis() - timeout > 95) {
            errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif
        }
        #endif
    }

    ce(LOW);               //Set STANDBY-I mode
    return 1;
}

/****************************************************************************/

bool RF24_Inkplate::txStandBy(uint32_t timeout, bool startTx)
{

    if (startTx) {
        stopListening();
        ce(HIGH);
    }
    uint32_t start = millis();

    while (!(read_register(FIFO_STATUS) & _BV(TX_EMPTY))) {
        if (status & _BV(MAX_RT)) {
            write_register(NRF_STATUS, _BV(MAX_RT));
            ce(LOW); // Set re-transmit
            ce(HIGH);
            if (millis() - start >= timeout) {
                ce(LOW);
                flush_tx();
                return 0;
            }
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (millis() - start > (timeout + 95)) {
            errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif
        }
        #endif
    }

    ce(LOW);  //Set STANDBY-I mode
    return 1;

}

/****************************************************************************/

void RF24_Inkplate::maskIRQ(bool tx, bool fail, bool rx)
{
    /* clear the interrupt flags */
    config_reg &= ~(1 << MASK_MAX_RT | 1 << MASK_TX_DS | 1 << MASK_RX_DR);
    /* set the specified interrupt flags */
    config_reg |= fail << MASK_MAX_RT | tx << MASK_TX_DS | rx << MASK_RX_DR;
    write_register(NRF_CONFIG, config_reg);
}

/****************************************************************************/

uint8_t RF24_Inkplate::getDynamicPayloadSize(void)
{
    uint8_t result = read_register(R_RX_PL_WID);

    if (result > 32) {
        flush_rx();
        delay(2);
        return 0;
    }
    return result;
}

/****************************************************************************/

bool RF24_Inkplate::available(void)
{
    return available(NULL);
}

/****************************************************************************/

bool RF24_Inkplate::available(uint8_t* pipe_num)
{
    // get implied RX FIFO empty flag from status byte
    uint8_t pipe = (get_status() >> RX_P_NO) & 0x07;
    if (pipe > 5)
        return 0;

    // If the caller wants the pipe number, include that
    if (pipe_num)
        *pipe_num = pipe;

    return 1;
}

/****************************************************************************/

void RF24_Inkplate::read(void* buf, uint8_t len)
{

    // Fetch the payload
    read_payload(buf, len);

    //Clear the only applicable interrupt flags
    write_register(NRF_STATUS, _BV(RX_DR));

}

/****************************************************************************/

void RF24_Inkplate::whatHappened(bool& tx_ok, bool& tx_fail, bool& rx_ready)
{
    // Read the status & reset the status in one easy call
    // Or is that such a good idea?
    write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

    // Report to the user what happened
    tx_ok = status & _BV(TX_DS);
    tx_fail = status & _BV(MAX_RT);
    rx_ready = status & _BV(RX_DR);
}

/****************************************************************************/

void RF24_Inkplate::openWritingPipe(uint64_t value)
{
    // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
    // expects it LSB first too, so we're good.

    write_register(RX_ADDR_P0, reinterpret_cast<uint8_t*>(&value), addr_width);
    write_register(TX_ADDR, reinterpret_cast<uint8_t*>(&value), addr_width);
}

/****************************************************************************/
void RF24_Inkplate::openWritingPipe(const uint8_t* address)
{
    // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
    // expects it LSB first too, so we're good.
    write_register(RX_ADDR_P0, address, addr_width);
    write_register(TX_ADDR, address, addr_width);
}

/****************************************************************************/
static const uint8_t child_pipe[] = {RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2,
                                             RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};

void RF24_Inkplate::openReadingPipe(uint8_t child, uint64_t address)
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
            write_register(child_pipe[child], reinterpret_cast<const uint8_t*>(&address), addr_width);
        } else {
            write_register(child_pipe[child], reinterpret_cast<const uint8_t*>(&address), 1);
        }

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        write_register(EN_RXADDR, read_register(EN_RXADDR) | _BV(child_pipe_enable[child]));
    }
}

/****************************************************************************/
void RF24_Inkplate::setAddressWidth(uint8_t a_width)
{

    if (a_width -= 2) {
        write_register(SETUP_AW, a_width % 4);
        addr_width = (a_width % 4) + 2;
    } else {
        write_register(SETUP_AW, 0);
        addr_width = 2;
    }

}

/****************************************************************************/

void RF24_Inkplate::openReadingPipe(uint8_t child, const uint8_t* address)
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
            write_register(child_pipe[child], address, addr_width);
        } else {
            write_register(child_pipe[child], address, 1);
        }

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        write_register(EN_RXADDR, read_register(EN_RXADDR) | _BV(child_pipe_enable[child]));

    }
}

/****************************************************************************/

void RF24_Inkplate::closeReadingPipe(uint8_t pipe)
{
    write_register(EN_RXADDR, read_register(EN_RXADDR) & ~_BV(child_pipe_enable[pipe]));
}

/****************************************************************************/

void RF24_Inkplate::toggle_features(void)
{
    beginTransaction();
    status = _SPI->transfer(ACTIVATE);
    _SPI->transfer(0x73);
    endTransaction();
}

/****************************************************************************/

void RF24_Inkplate::enableDynamicPayloads(void)
{
    // Enable dynamic payload throughout the system

    //toggle_features();
    write_register(FEATURE, read_register(FEATURE) | _BV(EN_DPL));

    // Enable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P5) | _BV(DPL_P4) | _BV(DPL_P3) | _BV(DPL_P2) | _BV(DPL_P1) | _BV(DPL_P0));

    dynamic_payloads_enabled = true;
}

/****************************************************************************/
void RF24_Inkplate::disableDynamicPayloads(void)
{
    // Disables dynamic payload throughout the system.  Also disables Ack Payloads

    //toggle_features();
    write_register(FEATURE, 0);

    // Disable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    write_register(DYNPD, 0);

    dynamic_payloads_enabled = false;
    ack_payloads_enabled = false;
}

/****************************************************************************/

void RF24_Inkplate::enableAckPayload(void)
{
    // enable ack payloads and dynamic payload features

    if (!ack_payloads_enabled){
        write_register(FEATURE, read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL));

        // Enable dynamic payload on pipes 0
        write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P0) | _BV(DPL_P1));
        dynamic_payloads_enabled = true;
        ack_payloads_enabled = true;
    }
}

/****************************************************************************/

void RF24_Inkplate::disableAckPayload(void)
{
    // disable ack payloads (leave dynamic payload features as is)
    if (ack_payloads_enabled){
        write_register(FEATURE, read_register(FEATURE) | ~_BV(EN_ACK_PAY));

        ack_payloads_enabled = false;
    }
}

/****************************************************************************/

void RF24_Inkplate::enableDynamicAck(void)
{
    //
    // enable dynamic ack features
    //
    //toggle_features();
    write_register(FEATURE, read_register(FEATURE) | _BV(EN_DYN_ACK));
}

/****************************************************************************/

bool RF24_Inkplate::writeAckPayload(uint8_t pipe, const void* buf, uint8_t len)
{
    if (ack_payloads_enabled){
        const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);

        write_payload(current, len, W_ACK_PAYLOAD | (pipe & 0x07));
        return !(status & _BV(TX_FULL));
    }
    return 0;
}

/****************************************************************************/

bool RF24_Inkplate::isAckPayloadAvailable(void)
{
    return available(NULL);
}

/****************************************************************************/

bool RF24_Inkplate::isPVariant(void)
{
    return _is_p_variant;
}

/****************************************************************************/

void RF24_Inkplate::setAutoAck(bool enable)
{
    if (enable){
        write_register(EN_AA, 0x3F);
    }else{
        write_register(EN_AA, 0);
        // accomodate ACK payloads feature
        if (ack_payloads_enabled){
            disableAckPayload();
        }
    }
}

/****************************************************************************/

void RF24_Inkplate::setAutoAck(uint8_t pipe, bool enable)
{
    if (pipe < 6) {
        uint8_t en_aa = read_register(EN_AA);
        if (enable) {
            en_aa |= _BV(pipe);
        }else{
            en_aa &= ~_BV(pipe);
            if (ack_payloads_enabled && !pipe){
                disableAckPayload();
            }
        }
        write_register(EN_AA, en_aa);
    }
}

/****************************************************************************/

bool RF24_Inkplate::testCarrier(void)
{
    return (read_register(CD) & 1);
}

/****************************************************************************/

bool RF24_Inkplate::testRPD(void)
{
    return (read_register(RPD) & 1);
}

/****************************************************************************/

void RF24_Inkplate::setPALevel(uint8_t level, bool lnaEnable)
{

    uint8_t setup = read_register(RF_SETUP) & 0xF8;

    if (level > 3) {                            // If invalid level, go to max PA
        level = (RF24_PA_MAX << 1) + lnaEnable; // +1 to support the SI24R1 chip extra bit
    } else {
        level = (level << 1) + lnaEnable;       // Else set level as requested
    }

    write_register(RF_SETUP, setup |= level);   // Write it to the chip
}

/****************************************************************************/

uint8_t RF24_Inkplate::getPALevel(void)
{

    return (read_register(RF_SETUP) & (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH))) >> 1;
}

/****************************************************************************/

uint8_t RF24_Inkplate::getARC(void)
{

    return read_register(OBSERVE_TX) & 0x0F;
}

/****************************************************************************/

bool RF24_Inkplate::setDataRate(rf24_datarate_e speed)
{
    bool result = false;
    uint8_t setup = read_register(RF_SETUP);

    // HIGH and LOW '00' is 1Mbs - our default
    setup &= ~(_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

    #if !defined(F_CPU) || F_CPU > 20000000
    txDelay = 280;
    #else //16Mhz Arduino
    txDelay=85;
    #endif
    if (speed == RF24_250KBPS) {
        // Must set the RF_DR_LOW to 1; RF_DR_HIGH (used to be RF_DR) is already 0
        // Making it '10'.
        setup |= _BV(RF_DR_LOW);
        #if !defined(F_CPU) || F_CPU > 20000000
        txDelay = 505;
        #else //16Mhz Arduino
        txDelay = 155;
        #endif
    } else {
        // Set 2Mbs, RF_DR (RF_DR_HIGH) is set 1
        // Making it '01'
        if (speed == RF24_2MBPS) {
            setup |= _BV(RF_DR_HIGH);
            #if !defined(F_CPU) || F_CPU > 20000000
            txDelay = 240;
            #else // 16Mhz Arduino
            txDelay = 65;
            #endif
        }
    }
    write_register(RF_SETUP, setup);

    // Verify our result
    if (read_register(RF_SETUP) == setup) {
        result = true;
    }
    return result;
}

/****************************************************************************/

rf24_datarate_e RF24_Inkplate::getDataRate(void)
{
    rf24_datarate_e result;
    uint8_t dr = read_register(RF_SETUP) & (_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

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

void RF24_Inkplate::setCRCLength(rf24_crclength_e length)
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
    write_register(NRF_CONFIG, config_reg);
}

/****************************************************************************/

rf24_crclength_e RF24_Inkplate::getCRCLength(void)
{
    rf24_crclength_e result = RF24_CRC_DISABLED;
    uint8_t AA = read_register(EN_AA);
    config_reg = read_register(NRF_CONFIG);

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

void RF24_Inkplate::disableCRC(void)
{
    config_reg &= ~_BV(EN_CRC);
    write_register(NRF_CONFIG, config_reg);
}

/****************************************************************************/
void RF24_Inkplate::setRetries(uint8_t delay, uint8_t count)
{
    write_register(SETUP_RETR, (delay & 0xf) << ARD | (count & 0xf) << ARC);
}

/****************************************************************************/
void RF24_Inkplate::startConstCarrier(rf24_pa_dbm_e level, uint8_t channel)
{
    stopListening();
    write_register(RF_SETUP, read_register(RF_SETUP) | _BV(CONT_WAVE) | _BV(PLL_LOCK));
    if (isPVariant()){
        setAutoAck(0);
        setRetries(0, 0);
        uint8_t dummy_buf[32];
        for (uint8_t i = 0; i < 32; ++i)
            dummy_buf[i] = 0xFF;

        // use write_register() instead of openWritingPipe() to bypass
        // truncation of the address with the current RF24_Inkplate::addr_width value
        write_register(TX_ADDR, reinterpret_cast<uint8_t*>(&dummy_buf), 5);
        flush_tx();  // so we can write to top level

        // use write_register() instead of write_payload() to bypass
        // truncation of the payload with the current RF24_Inkplate::payload_size value
        write_register(W_TX_PAYLOAD, reinterpret_cast<const uint8_t*>(&dummy_buf), 32);

        disableCRC();
    }
    setPALevel(level);
    setChannel(channel);
    ce(HIGH);
    if (isPVariant()){
        delay(1); // datasheet says 1 ms is ok in this instance
        ce(LOW);
        reUseTX();
    }
}

/****************************************************************************/
void RF24_Inkplate::stopConstCarrier()
{
    /*
     * A note from the datasheet:
     * Do not use REUSE_TX_PL together with CONT_WAVE=1. When both these
     * registers are set the chip does not react when setting CE low. If
     * however, both registers are set PWR_UP = 0 will turn TX mode off.
     */
    powerDown();  // per datasheet recommendation (just to be safe)
    write_register(RF_SETUP, read_register(RF_SETUP) & ~_BV(CONT_WAVE) & ~_BV(PLL_LOCK));
    ce(LOW);
}
