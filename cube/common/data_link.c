#include "data_link.h"
#include "address.h"


#ifndef SIMULATION
#include "trx.h"
#else
#include "sim_trx.h"
#endif


// frame[0] = frame length

// ---------------------------- NETWORKING INTERFACE ---------------------------

// This blocking function gets a payload from the data link layer
// and writes it to the buffer.
// It returns the length of the payload (i.e. the packet).
byte data_link_rx(byte* buffer, byte buf_len) {

    byte rxframe[TRX_PAYLOAD_LENGTH];
    trx_receive_payload(rxframe);
    for (int i = 0; i < buf_len && i < TRX_PAYLOAD_LENGTH - FRAME_HEADER_LEN; i++) {
        buffer[i] = rxframe[i+FRAME_HEADER_LEN];
    }

    return buffer[0] - FRAME_HEADER_LEN;
}

void data_link_tx(byte* payload, byte payload_len, uint32_t addr) {

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

    // TO-DO: handle return value
    trx_transmit_payload(addr, payload_zero_pad, TRX_PAYLOAD_LENGTH);

}