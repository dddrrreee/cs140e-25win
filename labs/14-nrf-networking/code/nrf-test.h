#ifndef __NRF_TEST_H__
#define __NRF_TEST_H__
// various support routines for testing.  
#include "rpi.h"
#include "nrf.h"

// left-side NRF pin-out of parthiv's board 
static inline nrf_conf_t
parthiv_left(uint32_t nbytes, uint32_t ch) {
    nrf_conf_t c = nrf_conf_mk(nbytes, ch);
    c.spi_chip = 0;  // the spi chip is 0.
    c.ce_pin = 6;
    c.int_pin = 22;
    return c;
}
// right side NRF pin-out of parthiv's board
static inline nrf_conf_t
parthiv_right(uint32_t nbytes, uint32_t ch) {
    nrf_conf_t c = nrf_conf_mk(nbytes, ch);
    c.spi_chip = 1;
    c.ce_pin = 5;
    c.int_pin = 23;
    return c;
}

// default: use parthiv right for client.
// if you want to use a different channel (e.g., b/c
// you keep getting interference, just change
// <nrf_default_channel> in <nrf-default-values.h>
static inline nrf_conf_t
client_conf(uint32_t nbytes) {
    return parthiv_right(nbytes,nrf_default_channel);
}
// default: use parthiv left for server.
static inline nrf_conf_t
server_conf(uint32_t nbytes) {
    return parthiv_left(nbytes,nrf_default_channel);
}

// server wrappers for making an nrf: 
//  - <ack_p> = 1, indicates packets should be ack'd 
//    (the hw will retransmit a small number of times).
//  - <ack_p> = 0, unacked packets.
static inline nrf_t *
server_mk(uint32_t rxaddr, uint32_t nbytes, int ack_p) {
    return nrf_init(server_conf(nbytes), rxaddr, ack_p);
}
static inline nrf_t *
server_mk_ack(uint32_t rxaddr, uint32_t nbytes) {
    return server_mk(rxaddr, nbytes, 1);
}
static inline nrf_t *
server_mk_noack(uint32_t rxaddr, uint32_t nbytes) {
    return server_mk(rxaddr, nbytes, 0);
}

// same as server, but for clients.
static inline nrf_t *
client_mk(uint32_t rxaddr, uint32_t nbytes, int ack_p) {
    return nrf_init(client_conf(nbytes), rxaddr, ack_p);
}
static inline nrf_t *
client_mk_ack(uint32_t rxaddr, uint32_t nbytes) {
    return client_mk(rxaddr, nbytes, 1);
}
static inline nrf_t *
client_mk_noack(uint32_t rxaddr, uint32_t nbytes) {
    return client_mk(rxaddr, nbytes, 0);
}
#endif
