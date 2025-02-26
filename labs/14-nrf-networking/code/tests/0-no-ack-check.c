// configure and dump hardware configuration.  get these working first.
#include "nrf-test.h"

void notmain(void) {
    unsigned nbytes = 32;
    kmalloc_init(1);

    trace("configuring no-ack server=[%x] with %d nbyte msgs\n", 
                server_addr, nbytes);

    // server uses the left nrf on parthiv's board, 
    //  - acked = 0, [no-ack]
    //  - tx/rx channel = <nrf-config.h:nrf_default_channel>
    //  - packet size=<nbytes>
    //  - <server_addr> is in <nrf-default-values.h>
    // see <nrf-test.h for definitions. 
    nrf_t *server_nic = server_mk_noack(server_addr, nbytes);
    nrf_dump("unreliable server config:\n", server_nic);

    trace("configuring no-ack client=[%x] with %d nbyte msg\n", 
                client_addr, nbytes);

    // client uses the right nrf on parthiv's board.
    //  - acked = 1,
    //  - tx/rx channel = <nrf-config.h:nrf_default_channel>
    //  - packet size=<nbytes>
    //  - <client_addr> is in <nrf-default-values.h>
    // see <nrf-test.h for definitions. 
    nrf_t *client_nic = client_mk_noack(client_addr, nbytes);
    nrf_dump("unreliable client config:\n", client_nic);

    // some simple combatibility checks.
    if(!nrf_compat(client_nic, server_nic))
        panic("did not configure correctly: not compatible\n");
}
