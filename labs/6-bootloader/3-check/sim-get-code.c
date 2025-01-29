// get-code (pi side) veneer.
#include "rpi.h"
#include "rpi-thread.h"

#include "sim.h"

// remap the interface to our fake versions so we can test.

// what is up w/ this?
static inline void (pi_side_put8)(src_loc_t src, uint8_t b) { 
    net_debug(LOG_PUTGET, "%s:%s:%d: pi put8: %x\n", 
            src.file, src.func, src.lineno, b);

    chan_push(&net.pi_to_unix, b);

    // whenever we push, yield to other thread.  slightly optimize
    // by waiting til its a multiple of 4.
    if(1 && chan_cnt(&net.pi_to_unix) % 4 == 0)
        rpi_yield();
}

static inline int pi_side_has_data(src_loc_t src) { 
    net_debug(LOG_PUTGET, "%s:%s:%d: pi has_data\n", 
            src.file, src.func, src.lineno);

    if(!chan_cnt(&net.unix_to_pi)) {
        // yield to unix side ~every 8th time.
        if(pi_random() % 8 == 0)
            rpi_yield();
    }

    return chan_cnt(&net.unix_to_pi);
}

static inline uint8_t (pi_side_get8)(src_loc_t loc) { 
    while(!pi_side_has_data(loc)) {
        if(net.unix_exited_p)
            panic("BUG: deadlock: pi side waiting on unix, which exited. :%s:%s:%d\n", loc.file,loc.file,loc.lineno);
    
        net_debug(LOG_FLOW, "pi no data going to yield\n");
        rpi_yield();
    }
    uint8_t b = chan_pop(&net.unix_to_pi);

    net_debug(LOG_PUTGET, "%s:%s:%d: pi get8 returning %x\n",
            loc.file, loc.func, loc.lineno,b);
    return b;
}
    
// the write to memory
static inline void    
pi_side_mem_put8(src_loc_t loc, uint32_t addr, uint8_t b)  { 
    net_debug(LOG_PUTGET, "%s:%s:%d: pi put8: %x = %x\n", 
        loc.file, loc.func, loc.lineno, addr, b);

    // omg: you didn't tell it the address to send it.
    uint8_t *p = (void*)addr;
    assert(p >= &net.recv[0] && p <= &net.recv[net.nbytes]);
    *p = b;
}

#include "src-loc.h"

static __attribute__((noreturn)) inline 
void pi_side_reboot(src_loc_t loc) {
    if(!net.pi_expect_reboot_p)
        panic("pi side rebooted unexpectedly at: %s:%s:%d\n",
            loc.file, loc.func, loc.lineno);

    net_debug(LOG_FLOW, "pi rebooting as expected!\n");
    net.pi_rebooted_p = net.pi_exited_p = 1;
    rpi_exit(0);
    not_reached();
}

uint32_t pi_side_timer_get_usec(void) {
    static uint32_t timer = 0;
    timer += 1000;
    // debug("timer=%d\n", timer);
    return timer;
}

// remap these three  routines.
#define rpi_reboot()    pi_side_reboot(SRC_LOC_MK())
#define clean_reboot()     rpi_reboot()
#define timer_get_usec  pi_side_timer_get_usec


#define PUT8(addr,x)            pi_side_mem_put8(SRC_LOC_MK(), addr, x)
#define boot_get8()     pi_side_get8(SRC_LOC_MK())
#define boot_put8(x)    pi_side_put8(SRC_LOC_MK(),x)
#define boot_has_data() pi_side_has_data(SRC_LOC_MK())

#include "get-code.h"

#undef rpi_reboot
#undef clean_reboot
#undef PUT8
#undef timer_get_usec 

// what's the cleanest way to run 
void pi_client(void *ctx) {
    net_debug(LOG_FLOW, "in pi thread\n");

    uint32_t addr = get_code();
    if(!addr)
        panic("failed\n");
    if(addr != (uint32_t)net.recv)
        panic("bug\n");

    net_debug(LOG_EXIT, "SIM: PI side: success, exiting!\n");
    net.pi_exited_p = 1;
    net.pi_success_p = 1;
    rpi_exit(0);
}
