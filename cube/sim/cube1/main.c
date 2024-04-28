/*
        WARNING

        This is a SIMULATION FILE.
        Do not compile this code for hardware.
*/


#include "sim_trx.h"
#include "sim_delay.h"
#include "transport.h"
#include "address.h"
#include "networking_constants.h"
#include <stdio.h>

int main() {

    char received_payload[200];
    for (int i = 0; i < 200; i++) {
        received_payload[i] = 0;
    }

    printf("::: Transport layer test :::\n");
    printf("::: Simulating cube 1    :::\n\n");

	trx_initialize(MY_DATA_LINK_ADDR);
	_delay_ms(300);

    while(1) {
        printf("Attempting to receive message... ");
        fflush(stdout);

        if (transport_rx(received_payload, 200)) {
            printf("\n\n===== Got something: ===== \n%s\n==========================\n\n", received_payload);
        }
        else {
            printf("Failed to get message.\n");
        }
    }

}
