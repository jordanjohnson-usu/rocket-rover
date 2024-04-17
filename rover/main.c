////////////////////////////////////////////////////////////////////////////////
//
// Rover Main
//
////////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "digital_io.h"
#include "uart.h"
#include "adc.h"
#include "motors.h"


int main() {

	digital_io_initialize();
	uart_initialize();
	adc_initialize();
	motors_initialize();

    while(1);

}
