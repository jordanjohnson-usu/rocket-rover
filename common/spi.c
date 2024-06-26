////////////////////////////////////////////////////////////////////////////////
//
// SPI
//
// Provides functions that allow the use of the ATMega's SPI peripheral.
// The ATMega is always set to be a SPI Master, but details of its operation can
// be configured via the provided macros. Only a single peripheral may be
// connected.
//
////////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>

#include "spi.h"

//////////////////// Private Defines ///////////////////////////////////////////

// The ports and pins of the SPI bus, including the SS/CS pin.
#define SPI_DDR         DDRB
#define SPI_PORT        PORTB
#define SPI_SS_INDEX    (2)
#define SPI_MOSI_INDEX  (3)
#define SPI_MISO_INDEX  (4)
#define SPI_SCLK_INDEX  (5)

#define SPI_DORD_MSB_FIRST (0)
#define SPI_DORD_LSB_FIRST ( _BV(DORD) )
#if SPI_DATA_ORDER == SPI_DATA_ORDER_MSB_FIRST
  #define SPI_DORD SPI_DORD_MSB_FIRST
#elif SPI_DATA_ORDER == SPI_DATA_ORDER_MSB_FIRST
  #define SPI_DORD SPI_DORD_LSB_FIRST
#else
  #define SPI_DATA_ORDER == SPI_DATA_ORDER_MSB_FIRST
#endif

#define SPI_CPOL_IDLE_LOW   (0)
#define SPI_CPOL_IDLE_HIGH  ( _BV(CPOL) )
#if SPI_CLOCK_POLARITY == SPI_CLOCK_POLARITY_IDLE_LOW
  #define SPI_CPOL SPI_CPOL_IDLE_LOW
#elif SPI_CLOCK_POLARITY == SPI_CLOCK_POLARITY_IDLE_HIGH
  #define SPI_CPOL SPI_CPOL_IDLE_HIGH
#else
  #define SPI_cpOL SPI_CPOL_IDLE_LOW
#endif

#define SPI_CPHA_SAMPLE_LEADING   (0)
#define SPI_CPHA_SAMPLE_TRAILING  ( _BV(CPHA) )
#if SPI_CLOCK_PHASE == SPI_CLOCK_PHASE_SAMPLE_LEADING
  #define SPI_CPHA SPI_CPHA_SAMPLE_LEADING
#elif SPI_CLOCK_PHASE == SPI_CLOCK_PHASE_SAMPLE_TRAILING
  #define SPI_CPHA SPI_CPHA_SAMPLE_TRAILING
#else
  #define SPI_CPHA SPI_CPHA_SAMPLE_LEADING
#endif

#define SPI_SPR_4     (0)
#define SPI_SPR_16    ( _BV(SPR0)             )
#define SPI_SPR_64    (             _BV(SPR1) )
#define SPI_SPR_128   ( _BV(SPR0) | _BV(SPR1) )
#define SPI_SPR_MASK  ( _BV(SPR1) | _BV(SPR0) )
#if SPI_CLOCK_IDEAL_PRESCALAR <= 4
  #define SPI_SPR (SPI_SPR_4 & SPI_SPR_MASK)
#elif SPI_CLOCK_IDEAL_PRESCALAR <= 16
  #define SPI_SPR (SPI_SPR_16 & SPI_SPR_MASK)
#elif SPI_CLOCK_IDEAL_PRESCALAR <= 64
  #define SPI_SPR (SPI_SPR_64 & SPI_SPR_MASK)
#elif SPI_CLOCK_IDEAL_PRESCALAR <= 128
  #define SPI_SPR (SPI_SPR_128 & SPI_SPR_MASK)
#else
  #define SPI_SPR (SPI_SPR_128 & SPI_SPR_MASK)
#endif

//////////////////// Static Variable Definitions ///////////////////////////////

// The message currently being received.
static spi_message_element_t receive_message_buffer[SPI_TRANSACTION_MAX_LENGTH];

// The message currently being transmitted.
static spi_message_element_t transmit_message_buffer[SPI_TRANSACTION_MAX_LENGTH];

// The index of the transaction bit that is currently being transmitted and received.
static spi_transaction_index_t transaction_index;

// The length of the current transaction.
static spi_transaction_length_t current_transaction_length;

// The callback that will be executed once the current transaction finishes.
static void (*current_transaction_complete_callback)(
  const spi_message_element_t *received_message,
  spi_transaction_length_t received_message_length
);

//////////////////// Public Function Bodies ////////////////////////////////////

// Initialize the SPI, including configuring the appropriate pins.
void spi_initialize(void) {

  // Configure the SPI pins appropriately.
  SPI_DDR |= (
    0
    | _BV(SPI_SCLK_INDEX) // Sets the SCK pin as an output.
    | _BV(SPI_MOSI_INDEX) // Sets the MOSI pin as an output.
    | _BV(SPI_SS_INDEX)   // Sets the SS pin as an output.
  );

  // Other pins (MISO) are inputs.
  SPI_DDR &= ~(
    0
    | _BV(SPI_MISO_INDEX)
  );

  SPI_PORT |= (
    0
    | _BV(SPI_SS_INDEX)   // Set the CS pin high.
    | _BV(SPI_MISO_INDEX) // Enable the pull-up on the MISO pin.
  );
  
  // SPCR is the only SPI register that needs to be configured.  
  SPCR = (
    0
    | _BV(SPE)    // Enables SPI.
    | SPI_DORD    // Configures data direction.
    | _BV(MSTR)   // Configures the SPI as a master.
    | SPI_CPOL    // Configures the SPI clock polarity.
    | SPI_CPHA    // Configures the SPI clock phase.
    | SPI_SPR     // Configures the SPI prescaler.
  );

}

// Transmits a message over the SPI. The response of the slave is placed in the
// buffer passed in as the first argument, beginning with given beginning index.
// Any elements before that index are discarded. The message to transmit is 
// composed of a number of "sections." Each section consists of a buffer, 
// containing the data to transmit, and an integer, respresenting the number of 
// bytes in the section. For example,
//
// spi_execute_transaction(response, 1, 2, section1, section1_length, section2, section2_length);
//
// The third argument gives the number of sections that compose the message. If
// a section is given as "NULL", that section is filled with 0x00.
void spi_execute_transaction(
  spi_message_element_t *response,
  spi_transaction_index_t response_beginning_index,
  uint8_t section_count,
  ...
) {

  va_list args;
  va_start(args, section_count);

  spi_message_element_t *section = va_arg(args, spi_message_element_t*);
  int section_length = va_arg(args, int);
  int section_index = 0;

  spi_transaction_index_t next_element_index = 0;
  spi_message_element_t next_element;
  spi_message_element_t received_element;

  // Select the device.
  SPI_PORT &= ~_BV(SPI_SS_INDEX);

  // The transaction will run no longer than the maximum length defined in the
  // header. However, in normal operation, this loop will be broken as soon as
  // the full message has been sent.
  for (spi_transaction_index_t i = 0; i < SPI_TRANSACTION_MAX_LENGTH; i++) {

    // If that was the last character of the section,
    if (next_element_index == section_length) {

      // If that was the last section,
      section_index = section_index + 1;
      if (section_index == section_count) {

        // The transaction is over.
        break;

      } else {

        // Move on to the next section.
        section = va_arg(args, spi_message_element_t*);
        section_length = va_arg(args, int);
        next_element_index = 0;

      }
    }

    if (section == NULL) {
      next_element = 0;
    } else {
      next_element = *(section + next_element_index);
    }

    SPDR = next_element;

    next_element_index = next_element_index + 1;

    while ((SPSR & _BV(SPIF)) == 0);

    received_element = SPDR;
    if ((response != NULL) && (i >= response_beginning_index)) {
      *(response + i - response_beginning_index) = received_element;
    }

  }

  // Release the device.
  SPI_PORT |= _BV(SPI_SS_INDEX);

  va_end(args);

}
