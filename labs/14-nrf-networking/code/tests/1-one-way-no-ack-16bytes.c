// one way test of 16-byte not-ack'd packets
#include "nrf-test.h"

// useful to mess around with these. 
enum { ntrial = 1000, timeout_usec = 1000 };


static void msg_set(uint32_t *m, unsigned i) {
    m[0] = i;
    m[1] = m[0]*m[0];
    m[2] = m[1]*m[1];
    m[3] = m[2]*m[2];
}
static int msg_chk(uint32_t *exp, uint32_t *got) {
    int ok_p = 1;
    for(int i = 0; i < 4; i++) {
        if(exp[i] != got[i]) {
            output("expected msg[%d] = %d, expected %d\n", 
                i,exp[i],got[i]);
            ok_p = 0;
        }
    }
    return ok_p;
}



// for simplicity the code is hardwired for 4 byte packets (the size
// of the data we send/recv).
static void
one_way_noack(nrf_t *server, nrf_t *client, int verbose_p) {
    unsigned client_addr = client->rxaddr;
    unsigned ntimeout = 0, npackets = 0;


    uint32_t rx[4], tx[4];

    // keep sending the loop variable to ourselves, and make sure we receive
    // it.
    for(unsigned i = 0; i < ntrial; i++) {
        // so we can see what kind of progress is being made.
        if(verbose_p && i  && i % 100 == 0)
            trace("sent %d no-ack'd packets\n", i);


        // send from server to client.
        msg_set(rx, i);
        nrf_send_noack(server, client_addr, rx, sizeof rx);

        // receive on client nic
        uint32_t x;
        if(nrf_read_exact_timeout(client, tx, sizeof tx, timeout_usec) == 16) {
            // we aren't doing acks, so can easily lose packets.  [i.e.,
            // it's not actually an error in the code.]
            if(!msg_chk(rx,tx)) {
                nrf_output("%d: lost/dup packet\n", i);
                client->tot_lost++;
            }
            npackets++;
        } else {
            if(verbose_p) 
                nrf_output("receive failed for packet=%d\n", i);
            ntimeout++;
        }
    }
    trace("trial: successfully sent %d no-ack'd pkts, [lost=%d, timeouts=%d]\n",
        npackets, client->tot_lost, ntimeout, ntimeout);
}

void notmain(void) {
    unsigned nbytes = 16;

    kmalloc_init(1);
    trace("configuring no-ack server=[%x] with %d nbyte msgs\n",
                server_addr, nbytes);
    // nrf-test.h
    nrf_t *s = server_mk_noack(server_addr, nbytes);
    nrf_dump("unreliable server config:\n", s);

    trace("configuring no-ack client=[%x] with %d nbyte msg\n",
                client_addr, nbytes);
    // nrf-test.h
    nrf_t *c = client_mk_noack(client_addr, nbytes);
    nrf_dump("unreliable client config:\n", c);

    // some simple combatibility checks.
    if(!nrf_compat(c, s))
        panic("did not configure correctly: not compatible\n");

    // reset the times so we get a bit better measurement.
    nrf_stat_start(s);
    nrf_stat_start(c);

    // do the test
    one_way_noack(s, c, 1);

    // print the stats.
    nrf_stat_print(s, "server: done with one-way test");
    nrf_stat_print(c, "client: done with one-way test");
}
