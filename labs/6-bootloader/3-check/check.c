#include "rpi.h"
#include "sim.h"

#if 0
#include "stack-T.h"
gen_stack_T(chan, chan_t, uint8_t, 256)
#else
#include "circular-T.h"
gen_circular_T(chan, chan_t, uint8_t, 256)
#endif

#include "pi-random.h"

enum { 
    LOG_PUTGET  = 1 << 0,       // all put/get calls
    LOG_EXIT    = 1 << 1,       // when code exits
    LOG_FLOW    = (1 << 2) | LOG_EXIT,  // flow of control.
    LOG_ALL     = ~0
};

int net_verbose_p = LOG_ALL; // LOG_FLOW;
#define net_debug(lvl, msg...) \
 do { if((lvl) & net_verbose_p) debug_output(msg); } while(0)

enum { max_nbytes = 1024 };

// simple network simulator state for our boot protocol.  usually
// would have generic network simulator state seperate from protocol-
// specific state, but we merge together to make things simple.
typedef struct {
    chan_t pi_to_unix;
    chan_t unix_to_pi;
    
    // 0 default for all.
    unsigned unix_exited_p:1,   // did unix side exit?
             unix_success_p:1;  // ... successfully?

    unsigned
             pi_exited_p:1,     // did the pi side exit?
             pi_success_p:1,    // .. successfully?
             pi_rebooted_p:1,   // did pi reboot (abort)?
             pi_expect_reboot_p:1;// did we expect a pi reboot (abort)
             ;

    unsigned unix_exit_on_byte_p:1;
    unsigned unix_exit_on_byte;
    unsigned unix_bytes_sent;

    uint8_t send[max_nbytes];
    uint8_t recv[max_nbytes];
    // actual message size for checking run: start small
    // to make debugging easier.
    unsigned nbytes;
} net_sim_t;


static net_sim_t net_mk(unsigned nbytes) {
    net_sim_t net = {0};

    net.pi_to_unix = chan_mk();
    net.unix_to_pi = chan_mk();
    net.nbytes = nbytes;
    for(unsigned i = 0; i < nbytes; i++)
        net.send[i] = pi_random();
    return net;
}

// should be in the thread state.
static net_sim_t net;


// bad form, but we do it like this so its easy to see in a short lab.
#include "sim-get-code.c"

#include "sim-put-code.c"

// check successful run of <nbytes>: if you do over and over
// will try different randomized values.
void check_run(unsigned nbytes) {
    net = net_mk(nbytes);
    rpi_fork(pi_client, &net)->annot = "pi side";
    rpi_fork(unix_server, &net)->annot = "unix side";
    rpi_thread_start();

    unsigned nerror = 0;
    for(unsigned i = 0; i < nbytes; i++) {
        if(net.send[i] != net.recv[i]) {
            trace("error: byte %d received is %x != sent %x\n",
                i, net.send[i], net.recv[i]);
            nerror++;
        }
    }
    if(nerror)
        panic("%d total errors\n", nerror);

    trace("SUCCESS: sent/received %d bytes.\n", nbytes);
}

// check failure run where the unix side goes away
// after N bytes sent (<exit_on_byte>).
void check_timeout_run(unsigned exit_on_byte, unsigned nbytes) {
    net = net_mk(nbytes);

    net.unix_exit_on_byte = exit_on_byte;
    net.unix_exit_on_byte_p = 1;
    // net.pi_expect_timeout_p = 1;

    rpi_fork(pi_client, &net)->annot = "pi side";
    rpi_fork(unix_server, &net)->annot = "unix side";
    rpi_thread_start();

    if(exit_on_byte >= net.unix_bytes_sent)
        panic("weird: should not drop after sent\n");

    if(!net.pi_rebooted_p)
        panic("pi did not detect error? [unix sent:%d bytes\n", net.unix_bytes_sent);
    output("pi rebooted as expected: byte %d\n", exit_on_byte);
    
}

void notmain(void) {
#if 0
    // setup boot: just send 4 bytes for ease of debugging.
    check_run(4);

    

    net_verbose_p = 0;
    for(int i = 0; i < 128; i++)
        check_run(i);
#endif

    net_verbose_p = 0;
    check_run(8);
//    net_verbose_p = 1;
        output("---------------------------------------------------\n");
    unsigned n = net.unix_bytes_sent-1;
    net_verbose_p = ~0;
#if 0
    int i = 5;
    output("dropping byte %d out of total bytes sent %d\n", i, n);
    check_timeout_run(i, 4);
#endif

    for(unsigned i = 5; i < n; i++) {
        output("---------------------------------------------------\n");
        output("dropping byte %d out of total bytes sent %d\n", i, n);
        check_timeout_run(i, 4);
    }
}
