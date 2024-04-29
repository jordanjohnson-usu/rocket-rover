#include "data_link.h"
#include "address.h"
#include "uart.h"


#ifndef SIMULATION
#include "trx.h"
#else
#include "sim_trx.h"
#include <stdio.h>
#endif


// frame[0] = frame length

// ---------------------------- NETWORKING INTERFACE ---------------------------

// This blocking function gets a payload from the data link layer
// and writes it to the buffer.
// It returns if it was successful (false if timed out).
bool data_link_rx(byte* buffer, byte buf_len, timer_delay_ms_t timeout_ms) {

    byte rxframe[TRX_PAYLOAD_LENGTH];
    trx_reception_outcome_t outcome = trx_receive_payload(rxframe, timeout_ms);
    if (outcome == TRX_RECEPTION_FAILURE) {
        return false;
    }


    for (int i = 0; i < buf_len && i < TRX_PAYLOAD_LENGTH - FRAME_HEADER_LEN; i++)
        buffer[i] = rxframe[i+FRAME_HEADER_LEN];

    return true;
}


bool data_link_tx(byte* payload, byte payload_len, uint32_t addr) {

    uart_transmit_formatted_message("data_link_tx: Transmitting to %08lx\r\n", addr);

    trx_transmission_outcome_t success;

    // initialize
    byte payload_zero_pad[TRX_PAYLOAD_LENGTH];
    for (int i = 0; i < TRX_PAYLOAD_LENGTH; i++) {
        payload_zero_pad[i] = 0;
    }

    // set length
    payload_zero_pad[0] = payload_len;

    // fill payload
    for (int i = 0; i < payload_len && i < TRX_PAYLOAD_LENGTH - FRAME_HEADER_LEN; i++) {
        payload_zero_pad[i + FRAME_HEADER_LEN] = payload[i];
    }

    success = trx_transmit_payload(addr, payload_zero_pad, TRX_PAYLOAD_LENGTH);
    if (success == TRX_TRANSMISSION_SUCCESS) {
        return true;
    }
    else {
        return false;
    }

}
