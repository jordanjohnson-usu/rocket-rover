/*
        WARNING

        This is a SIMULATION FILE.
        Do not compile this code for hardware.

*/


#include "sim_trx.h"
#include "sim_delay.h"
#include "network.h"
#include "address.h"
#include <stdio.h>

int main() {

    char received_payload[256];
    for (int i = 0; i < 256; i++) {
        received_payload[i] = 0;
    }

    printf("Cube 2 Powering on...\n");

	trx_initialize(MY_DATA_LINK_ADDR);
	_delay_ms(300);

    for (int i = 0; i < TRX_PAYLOAD_LENGTH + 1; i++) {
        received_payload[i] = '\0';
    }

    while(1) {
        printf("Attempting to receive packet...\n");
        network_rx(received_payload, MAX_PACKET_LEN);
        printf("Got something: %s\n\n", received_payload);
        _delay_ms(1000);
    }

}