/*
    Welcome to a bit of a bizarre transport layer.
    It only works with one message sender at a time. This is because we only
    expect one sender at a time. A more correct transport layer is more
    complicated and a little impractical for our tiny microcontroller.

    This transport layer abuses the "port" concept. Normally, an endpoint
    is uniquely identified by a network address and port number pair.
    In this protocol stack, the port is globally unique, and therefore
    the port number does not need to be paired with the network address.
*/

#include <stdlib.h>
#include "address_resolution.h"
#include "transport.h"
#include "network.h"
#include "address.h"
#include "cube_parameters.h"

#ifndef SIMULATION
#include "trx.h"
#include <util/delay.h>
#else
#include "sim_trx.h"
#include "sim_delay.h"
#include "print_data.h"
#include <stdio.h>
#endif

// the transport layer intentionally adds delays between segments.
// fun fact: in the TCP/IP protocol stack,
// the transport layer is actually responsible for preventing
// a network from being overwhelmed by throttling its own output!


#define TRANSPORT_TX_ACK_TIMEOUT_MS (1500)
#define TRANSPORT_TX_ACK_DELAY_MS (250)
#define TRANSPORT_TX_SEGMENT_SPACING_MS (250)
#define TRANSPORT_TX_RETRY_DELAY_MS (250)
#define TRANSPORT_TX_ATTEMPT_LIMIT (10)


// four segment types: START_OF_MESSAGE, DATA, END_OF_MESSAGE, ACK

// START_OF_MESSAGE segment:
// segment[0] = length of segment = 6
// segment[1] = sequence number
// segment[2] = destination port number
// segment[3] = source port number
// segment[4] = segment identifier = 0x07, START_OF_MESSAGE
// segment[5-6] = total length of message

// DATA segment:
// segment[0] = length of segment
// segment[1] = sequence number
// segment[2] = destination port number
// segment[3] = source port number
// segment[4] = segment identifier = 0x0D, DATA
// segment[5-6] = start address (starting memory address of this segment's data)
// rest is payload

// END_OF_MESSAGE segment:
// segment[0] = length of segment = 5
// segment[1] = sequence number
// segment[2] = destination port number
// segment[3] = source port number
// segment[4] = segment identifier = 0x09, END_OF_MESSAGE

// ACK segment:
// segment[0] = length of segment = 5
// segment[1] = sequence number
// segment[2] = destination port number
// segment[3] = source port number
// segment[4] = segment identifier = 0x0A, ACK


// Using an enum for a "state machine" to make this a little more easily expandable.
enum rx_state_t {
    RXST_Idle,
    RXST_Receiving
};



/*
================================================================================

            Theory

=== Receiver side ===
if I receive a frame, I first acknowledge it no matter what.
If the frame's sequence number is 0, I ack 1, regardless of what my expected_seq_num is.
If the frame's sequence number is 1, I ack 0, regardless of what my expected_seq_num is.

If the frame's sequence number matches my "expected_seq_num", we're golden.
I'll do something with this frame. I will also advance my expected_seq_num.
If it does not match... well, somehow my previous ack got lost in transmission,
so the other guy sent the same thing again. I won't do anything this time.
Hopefully he got my ack this time and sends the next frame.

=== Transmitter side ===
We both start at seq number 0. This will be my "current seq num".

I will send a message. Then I'll immediately start waiting for an ack.
If I timeout, I will re-send my message.
If I get an ack, but the ack's sequence number matches mine,
I will re-send my message.
If I get an ack, and the ack sequence number is advanced one,
I can finally adjust "current seq number" and move on and send the next packet.

================================================================================
*/




// ======================= Receiver Code =======================================

typedef enum {
    TRANSPORT_ATTEMPT_RX_SUCCESS,
    TRANSPORT_ATTEMPT_RX_OUTDATED,
    TRANSPORT_ATTEMPT_RX_TIMEOUT,
    TRANSPORT_ATTEMPT_RX_ERROR
} transport_attempt_rx_result;

// Get data from the network layer.
// Acknowledge it.
// Verify the data is new by checking the sequence number.
// Returns true if the reception was successful.
transport_attempt_rx_result transport_attempt_rx(byte* segment, byte buf_len, uint16_t timeout_ms) {

    static byte rx_seq = 0; // What we expect the next sequence number to be.
    network_rx_result result;
    byte ack_seg[ACK_SEGMENT_HEARDER_LEN];

    // try to receive some data from the network
    result = network_rx(segment, MAX_SEGMENT_LEN, timeout_ms);
    if (result == NETWORK_RX_TIMEOUT) return TRANSPORT_ATTEMPT_RX_TIMEOUT;
    if (result == NETWORK_RX_ERROR) return TRANSPORT_ATTEMPT_RX_ERROR;

    // If we got a START_OF_MESSAGE, we must synchronize our sequence numbers.
    if (segment[4] == SEGID_START_OF_MESSAGE) rx_seq = segment[1];

    // Alright we got something, let me acknowledge it really quick.
    _delay_ms(TRANSPORT_TX_ACK_DELAY_MS);
    ack_seg[0] = ACK_SEGMENT_HEARDER_LEN;
    ack_seg[1] = segment[1] == 0 ? 1 : 0; // advance seq number
    ack_seg[2] = segment[3]; // destination port = port of whoever sent
    ack_seg[3] = MY_PORT;    // source port = me :)
    ack_seg[4] = SEGID_ACK;
    // if this errors out, we don't care, the other guy will send me another thing anyways
    network_tx(ack_seg, ACK_SEGMENT_HEARDER_LEN, resolve_network_addr(segment[3]), MY_NETWORK_ADDR);

    // Okay. Is this new data?
    if (rx_seq != segment[1]) {
        return TRANSPORT_ATTEMPT_RX_OUTDATED;
    }

    // Great, new data. Let's advance our expected sequence number.
    rx_seq = rx_seq == 0 ? 1 : 0;

    return TRANSPORT_ATTEMPT_RX_SUCCESS;
}

typedef enum {
    TRANSPORT_KEEP_TRYING_TO_RX_SUCCESS,
    TRANSPORT_KEEP_TRYING_TO_RX_TIMEOUT,
    TRANSPORT_KEEP_TRYING_TO_RX_ERROR,
} transport_keep_trying_to_rx_result;

transport_keep_trying_to_rx_result transport_keep_trying_to_rx(byte* segment, byte buf_len, uint16_t timeout_ms) {
    while(true) {
        transport_attempt_rx_result result = transport_attempt_rx(segment, buf_len, timeout_ms);
        switch (result) {

        case TRANSPORT_ATTEMPT_RX_SUCCESS:
            return TRANSPORT_KEEP_TRYING_TO_RX_SUCCESS;
            break;

        case TRANSPORT_ATTEMPT_RX_OUTDATED:
            // Try again, please.
            break;

        case TRANSPORT_ATTEMPT_RX_TIMEOUT:
            return TRANSPORT_KEEP_TRYING_TO_RX_TIMEOUT;
            break;

        case TRANSPORT_ATTEMPT_RX_ERROR:
            // Actually... try again, please.
            //return TRANSPORT_KEEP_TRYING_TO_RX_ERROR;
            break;
        }

    }
}


// The application layer calls this function.
// Get a complete message.
// The function returns if a complete message was successfully received.
// The function will write to message_len to identify the length of the message.
// The function will write to source_port to identify who sent the message.

// NOTE: this system ONLY works with one transmitter at a time.
transport_rx_result transport_rx(byte* buffer, uint16_t buf_len, uint16_t* message_len, byte* source_port, uint16_t timeout_ms) {

    // Start by initializing the recepient's buffer to zero.
    for (int i = 0; i < buf_len; i++) buffer[i] = 0;

    int state = RXST_Idle;

    byte segment_len;
    byte segment[MAX_SEGMENT_LEN];

    byte segment_identifier;

    // Continually receive segments until we've put together a whole message.
    while(true) {

        // Get the next segment.
        // As a side effect, acknowledge anything we receive.
        transport_keep_trying_to_rx_result result = transport_keep_trying_to_rx(segment, MAX_SEGMENT_LEN, timeout_ms);
        if (result == TRANSPORT_KEEP_TRYING_TO_RX_TIMEOUT) return TRANSPORT_RX_TIMEOUT;
        if (result == TRANSPORT_KEEP_TRYING_TO_RX_ERROR) return TRANSPORT_RX_ERROR;

        segment_len = segment[0];
        segment_identifier = segment[4];

        switch(state) {

        case RXST_Idle:
            if (segment_identifier == SEGID_START_OF_MESSAGE) {
                if (source_port != NULL) {
                    *source_port = segment[3];
                }
                if (message_len != NULL) {
                    *message_len = segment[5] << 8 + segment[6];
                }
                state = RXST_Receiving;
            }
            break;

        case RXST_Receiving:
            if (segment_identifier == SEGID_DATA) {
                uint16_t offset = (segment[5] << 8) + segment[6];
                byte payload_len = segment_len - DATA_SEGMENT_HEADER_LEN;

                for (uint16_t i = 0; i < payload_len && i + offset < buf_len; i++) {
                    buffer[i + offset] = segment[i + DATA_SEGMENT_HEADER_LEN];
                }
            }
            else if (segment_identifier == SEGID_END_OF_MESSAGE) {
                state = RXST_Idle;
                return TRANSPORT_RX_SUCCESS;
            }
            // This state is possible if the message fails and the other guy
            // tries again from the beginning.
            else if (segment_identifier == SEGID_START_OF_MESSAGE) {
                if (message_len != NULL) {
                    *message_len = segment[5] << 8 + segment[6];
                }
            }
            break;
        }
    }
}



// ======================= Transmitter Code ====================================

typedef enum {
    TRANSPORT_ATTEMPT_TX_SUCCESS,
    TRANSPORT_ATTEMPT_TX_TRANSMIT_FAILED,
    TRANSPORT_ATTEMPT_TX_NOT_ACKNOWLEDGED,
    TRANSPORT_ATTEMPT_TX_OLD_ACK,
    TRANSPORT_ATTEMPT_TX_NOT_AN_ACK,
    TRANSPORT_ATTEMPT_TX_ERROR
} transport_attempt_tx_result;

// This function transmits a segment, then waits to receive an acknowledgement.
// This function can time out.
// The function returns whether the acknowledgement was received before the timeout.
transport_attempt_tx_result transport_attempt_tx(byte* segment, byte segment_len, byte dest_port, byte current_seq_num) {

    byte hopefully_an_ack[ACK_SEGMENT_HEARDER_LEN];
    network_tx_result tx_result;

    // Let's send this bad boy.
    tx_result = network_tx(segment, segment_len, resolve_network_addr(dest_port), MY_NETWORK_ADDR);

    // Why is this commented out?
    // For some reason, we sometimes get errors, even when the transmission is successful.
    // We should just rely on the transport layer acknowledgement. It acts more predictably.
    //if (tx_result == NETWORK_TX_FAILURE) {
        //_delay_ms(TRANSPORT_TX_ACK_TIMEOUT_MS);
        //return TRANSPORT_ATTEMPT_TX_TRANSMIT_FAILED;
    //}

    // Now let's try to get an acknowledgement.
    network_rx_result rx_result = network_rx(hopefully_an_ack, ACK_SEGMENT_HEARDER_LEN, TRANSPORT_TX_ACK_TIMEOUT_MS);
    if (rx_result == NETWORK_RX_TIMEOUT) return TRANSPORT_ATTEMPT_TX_NOT_ACKNOWLEDGED;
    if (rx_result == NETWORK_RX_ERROR) return TRANSPORT_ATTEMPT_TX_ERROR;
    if (hopefully_an_ack[4] != SEGID_ACK) return TRANSPORT_ATTEMPT_TX_NOT_AN_ACK;
    if (hopefully_an_ack[1] == current_seq_num) return TRANSPORT_ATTEMPT_TX_OLD_ACK;

    return TRANSPORT_ATTEMPT_TX_SUCCESS;
}

typedef enum {
    TRANSPORT_KEEP_TRYING_TO_TX_SUCCESS,
    TRANSPORT_KEEP_TRYING_TO_TX_REACHED_ATTEMPT_LIMIT,
    TRANSPORT_KEEP_TRYING_TO_TX_ERROR
} transport_keep_trying_to_tx_result;


// This function continually tries to transmit segments until one is acknowledged.
// This function can fail if the attempted transmissions exceeds TRANSPORT_TX_ATTEMPT_LIMIT.
// The function returns whether the segment was eventually transmitted and acknowledged.
transport_keep_trying_to_tx_result transport_keep_trying_to_tx(byte* segment, byte segment_len, byte dest_port, byte current_seq_num) {

    uint16_t transmit_attempts = 0;
    transport_attempt_tx_result result;

    while(true) {

        transmit_attempts++;
        if (transmit_attempts > TRANSPORT_TX_ATTEMPT_LIMIT) {
            return TRANSPORT_KEEP_TRYING_TO_TX_REACHED_ATTEMPT_LIMIT;
        }

        result = transport_attempt_tx(segment, segment_len, dest_port, current_seq_num);

        if (result == TRANSPORT_ATTEMPT_TX_SUCCESS) return TRANSPORT_KEEP_TRYING_TO_TX_SUCCESS;
        if (result == TRANSPORT_ATTEMPT_TX_ERROR) return TRANSPORT_KEEP_TRYING_TO_TX_ERROR;

        // If I get Not an Ack, Old Ack, or Not Acknowledged, let's try again.

        _delay_ms(TRANSPORT_TX_RETRY_DELAY_MS);

    }
}

// The application layer calls this function.
// The function takes the message, splits it up into segments,
// and sends each segment one-by-one.
// Every segment must be acknowledged before the next one is sent.
// The function can fail if one of the segments is not acknowledged in time.
// The function returns whether the message was sent successfully.
transport_tx_result transport_tx(byte* message, uint16_t message_len, byte dest_port) {

    byte current_seq_num = 0;

    byte segment[MAX_SEGMENT_LEN];
    uint16_t bytes_remaining = message_len;

    transport_keep_trying_to_tx_result result;

    // ------ send START_OF_MESSAGE -----
    segment[0] = START_SEGMENT_HEADER_LEN;
    segment[1] = current_seq_num;
    segment[2] = dest_port;
    segment[3] = MY_PORT;
    segment[4] = SEGID_START_OF_MESSAGE;
    segment[5] = (message_len & 0xFF00) >> 8;
    segment[6] = (message_len & 0x00FF) >> 0;

    result = transport_keep_trying_to_tx(segment, START_SEGMENT_HEADER_LEN, dest_port, current_seq_num);
    if (result == TRANSPORT_KEEP_TRYING_TO_TX_REACHED_ATTEMPT_LIMIT) return TRANSPORT_TX_REACHED_ATTEMPT_LIMIT;
    if (result == TRANSPORT_KEEP_TRYING_TO_TX_ERROR) return TRANSPORT_TX_ERROR;
    current_seq_num = current_seq_num == 0 ? 1 : 0;

    _delay_ms(TRANSPORT_TX_SEGMENT_SPACING_MS);

    // ------ send data segments -----
    while (bytes_remaining > 0) {

        // can't send the whole darn thing at once unless it's small
        byte this_payload_len;
        if (bytes_remaining > MAX_SEGMENT_LEN - DATA_SEGMENT_HEADER_LEN) {
            this_payload_len = MAX_SEGMENT_LEN - DATA_SEGMENT_HEADER_LEN;
        }
        else {
            this_payload_len = (byte) bytes_remaining;
        }

        byte this_segment_len = this_payload_len + DATA_SEGMENT_HEADER_LEN;
        uint16_t start_address = message_len - (uint16_t) bytes_remaining;

        segment[0] = this_segment_len;
        segment[1] = current_seq_num;
        segment[2] = dest_port;
        segment[3] = MY_PORT;
        segment[4] = SEGID_DATA;
        segment[5] = (start_address & 0xFF00) >> 8;
        segment[6] = (start_address & 0x00FF) >> 0;
        for (byte i = 0; i < this_payload_len; i++) {
            segment[i + DATA_SEGMENT_HEADER_LEN] = message[i + start_address];
        }

        bytes_remaining -= (uint16_t) this_payload_len;

        result = transport_keep_trying_to_tx(segment, this_segment_len, dest_port, current_seq_num);
        if (result == TRANSPORT_KEEP_TRYING_TO_TX_REACHED_ATTEMPT_LIMIT) return TRANSPORT_TX_REACHED_ATTEMPT_LIMIT;
        if (result == TRANSPORT_KEEP_TRYING_TO_TX_ERROR) return TRANSPORT_TX_ERROR;
        current_seq_num = current_seq_num == 0 ? 1 : 0;

        _delay_ms(TRANSPORT_TX_SEGMENT_SPACING_MS);
    }

    // ------ send END_OF_MESSAGE -----
    segment[0] = END_SEGMENT_HEADER_LEN;
    segment[1] = current_seq_num;
    segment[2] = dest_port;
    segment[3] = MY_PORT;
    segment[4] = SEGID_END_OF_MESSAGE;

    result = transport_keep_trying_to_tx(segment, END_SEGMENT_HEADER_LEN, dest_port, current_seq_num);
    if (result == TRANSPORT_KEEP_TRYING_TO_TX_REACHED_ATTEMPT_LIMIT) return TRANSPORT_TX_REACHED_ATTEMPT_LIMIT;
    if (result == TRANSPORT_KEEP_TRYING_TO_TX_ERROR) return TRANSPORT_TX_ERROR;
    current_seq_num = current_seq_num == 0 ? 1 : 0;

    return TRANSPORT_TX_SUCCESS;

}
