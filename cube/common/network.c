#include "networking_constants.h"
#include "address_resolution.h"
#include "network.h"
#include "data_link.h"

// packet[0] = length of packet
// packet[1] = final destination network address
// packet[2] = original source network address
// rest is payload


// Oh! This function will need to be different for EVERY data cube.
// Maybe we can throw this under some ifdef's and have some fun with
// our makefile. Until then, here's an example.
int next_hop(int final_addr) {
    int next_hop_addr;
    switch(final_addr) {
    case 0x0A:
        next_hop_addr = 0x0B;
        break;
    case 0x0B:
        next_hop_addr = 0x0B;
        break;
    case 0x0C:
        next_hop_addr = 0x0B;
        break;
    case 0x0D:
        next_hop_addr = 0x0B;
        break;
    }
    return next_hop_addr;
}

// This blocking function gets a payload from the network layer
// and writes it to the buffer.
// It returns the length of the payload (i.e. the segment).
int network_rx(char* buffer, int buf_len) {

    int packet_len;
    char packet[MAX_PACKET_LEN];

    // Repeat this until we get something that's for me.
    do {

        data_link_rx(packet, MAX_PACKET_LEN);
        packet_len = packet[0];

        // If packet isn't for me, forward it along.
        if (packet[1] != MY_NETWORK_ADDR) {
            network_tx(packet, packet_len, packet[1], packet[2]);
        }
        

    } while (packet[1] != MY_NETWORK_ADDR);

    // Got something for me. Let's return it.
    for (int i = 0; i < packet_len - PACKET_HEADER_LEN && i < buf_len; i++) {
        buffer[i] = packet[i + PACKET_HEADER_LEN];
    }

    return packet_len - PACKET_HEADER_LEN;
}

// Transmit to the specified network address.
void network_tx(char* payload, int payload_len, int dest_network_addr, int src_network_addr) {

    int packet_len = payload_len + PACKET_HEADER_LEN;
    char packet[packet_len];

    packet[0] = packet_len;
    packet[1] = dest_network_addr;
    packet[2] = src_network_addr;
    for (int i = 0; i < payload_len && i < MAX_PACKET_LEN - PACKET_HEADER_LEN; i++) {
        packet[i + PACKET_HEADER_LEN] = payload[i];
    }
    int next_hop_addr = next_hop(dest_network_addr);

    data_link_tx(packet, packet_len, resolve_data_link_addr(next_hop_addr));

    return;
}
