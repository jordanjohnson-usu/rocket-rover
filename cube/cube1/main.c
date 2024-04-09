/* * * * * * * * * * * * * * *
      Data Cube Software
 * * * * * * * * * * * * * * */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "spi.h"
#include "trx.h"
#include "application.h"

#include "cube_parameters.h"
#include <util/delay.h>


#include "address.h"
#define TARGET_ADDRESS	0x0A0A0A0A

static uart_message_element_t received_data;

static char* restart_message_format = "\n\rHello World.\n\r";
static char* status_message_format = "Transceiver has a status of %02x.\n\r";
static char* received_payload_format = "Received wireless message:\n\r%s\n\r";

static trx_payload_element_t *example_payload = "I hope this works, cuz if not...";

int main() {

	// Let's wait for all of our hardware to turn on.
	_delay_ms(1000);

	uart_initialize();
	uart_transmit_formatted_message(restart_message_format);
	UART_WAIT_UNTIL_DONE();

	_delay_ms(100);
	trx_initialize(MY_DATA_LINK_ADDR);
	_delay_ms(100);
	trx_transmit_payload(
		TARGET_ADDRESS, 
		example_payload, 
		TRX_PAYLOAD_LENGTH
	);

	while(1) {

		_delay_ms(100);
		trx_payload_element_t received_payload[TRX_PAYLOAD_LENGTH];
		trx_receive_payload(received_payload);
		uart_transmit_formatted_message(
			received_payload_format,
			received_payload
		);

	}

}
