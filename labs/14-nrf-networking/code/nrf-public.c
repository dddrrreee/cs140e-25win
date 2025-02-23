// public interface for the nrf driver.
#include "nrf.h"
#include "timeout.h"

// returns the number of bytes available.
//
// pulls data off if it's on the interface (since there can't be that much
// and we'd start dropping stuff).
int nrf_nbytes_avail(nrf_t *n) {
    // we aren't interrupt based atm, so always try to pull stuff
    // off the rx queue if we can.
    while(nrf_get_pkts(n))
        ;
    return cq_nelem(&n->recvq);
}

// non-blocking read from nrf.
//  - if there is less than <nbytes> of data on the receive queue
//    <recvq> return 0 immediately.  
//  - otherwise read <nbytes> of data into <msg> and return <nbytes>.
int nrf_read_exact_noblk(nrf_t *nic, void *msg, unsigned nbytes) {
    assert(nbytes > 0);

    unsigned n;
    if((n = nrf_nbytes_avail(nic)) < nbytes)
        return 0;

    // have at least nbytes of data: pull off of circular queue.
    cq_pop_n(&nic->recvq, msg, nbytes);
    return nbytes;
}

// read exactly <nbytes> of data: return the number of bytes
// read or -1 (timeout).
int nrf_read_exact_timeout(nrf_t *nic, 
    void *msg, 
    unsigned nbytes, 
    unsigned usec_timeout) 
{
    timeout_t t = timeout_start();
    int n;
    while(!(n = nrf_read_exact_noblk(nic, msg, nbytes))) {
        if(timeout_usec(&t, usec_timeout))
            return -1;

        // yield so that we can use threads and loopback on 
        // one machine.
        rpi_wait();
    }
    return n;
}

enum { NRF_TIMEOUT = 10};

// blocking: read exactly <nbytes> of data.
int nrf_read_exact(nrf_t *nic, void *msg, unsigned nbytes) {
    // we still "timeout" with a large value --- if we don't
    // get data for this long, there is some kind of issue,
    // so report an error.
    unsigned usec = NRF_TIMEOUT * 1000 * 1000;
    int n = nrf_read_exact_timeout(nic, msg, nbytes, usec);
    if(n == nbytes) 
        return n;

    // some kind of error.
    // nrf_dump("timeout config\n");
    panic("addr=%x:ret=%d: connection error: no traffic after %d seconds\n",
                    nic->rxaddr, n, NRF_TIMEOUT);
}

// we should have a tx structure that gives the size of the 
// actual receipient.

// for our tests, the sender and the receiver are setup the same,
// but in general you won't be able to know from the sender if
// <txaddr>:
//   - is setup for ack or no-ack
//   - the message size.
// in a full system we'd have a connection that contains all of this
// but for a short, lab, we just expose things (can build on top
// as an extension!).
int nrf_send_ack(nrf_t *nic, uint32_t txaddr, 
                const void *msg, unsigned nbytes) {
    return nrf_tx_send_ack(nic, txaddr, msg, nbytes);
}
int nrf_send_noack(nrf_t *nic, uint32_t txaddr, 
                const void *msg, unsigned nbytes) {
    return nrf_tx_send_noack(nic, txaddr, msg, nbytes);
}
