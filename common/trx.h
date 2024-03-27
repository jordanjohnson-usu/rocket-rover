#ifndef _TRX_H
#define _TRX_H

#include <avr/io.h>
#include <avr/interrupt.h>

////////////////////////////////////////////////////////////////////////////////
//
// TRX
//
// Provides functions for interfacing with the nRF24L01+ wireless transceiver.
// These functions use the ATMega's SPI. While using these functions, the SPI
// may not be used for any other purpose. The settings in spi.h are assumemd to
// be correct for driving the transceiver.
//
////////////////////////////////////////////////////////////////////////////////

/////////////////// TRX Settings ///////////////////////////////////////////////

// The length of payloads transmitted and received by this transceiver.
#define TRX_PAYLOAD_LENGTH (32)

// Payloads shorter than TRX_PAYLOAD_LENGTH are padded with this before being
// transmitted.
#define TRX_PAYLOAD_PADDING (0x00)

// The receiver address of this transceiver.
#define TRX_THIS_RX_ADDRESS (0xDEADBEEF)

// The ports and pins used to drive the chip-enable pin of the transceiver.
#define TRX_CE_DDR    DDRB
#define TRX_CE_PORT   PORTB
#define TRX_CE_PIN    PINB
#define TRX_CE_INDEX  (1)

/////////////////// TRX type definitions ///////////////////////////////////////

// Addresses are 32 bits wide, or 4 bytes.
typedef uint32_t trx_address_t;

// Payloads are transmitted one byte at a time.
typedef uint8_t trx_payload_element_t;

// The type of the transceiver's status register.
typedef uint8_t trx_status_buffer_t;

/////////////////// Public function prototypes /////////////////////////////////

// Initializes the TRX, including initializing the SPI and any other peripherals
// required.
void trx_initialize();

// Transmits a payload to the given address.
void trx_transmit_payload(
  trx_address_t address,
  trx_payload_element_t payload,
  int payload_length
);

// Gets the value currently in the status buffer. This is equivalent to what was
// in the transceiver's status register at the beginning of the last SPI
// transaction.
trx_status_buffer_t trx_get_status();

#endif