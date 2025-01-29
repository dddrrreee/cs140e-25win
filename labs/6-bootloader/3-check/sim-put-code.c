// put-code (unix side) veneer.
#include "rpi.h"
#include "sim.h"

void put_uint8(int fd, uint8_t b) { 
    net_debug(LOG_PUTGET, "unix put8: %x\n", b);
    chan_push(&net.unix_to_pi, b);

    unsigned n = net.unix_bytes_sent++;
    if(net.unix_exit_on_byte_p 
    && net.unix_exit_on_byte == n) {
        net_debug(LOG_FLOW, "UNIX: ABORT: early exit on byte %d\n", n);
        net.unix_exited_p = 1;
        net.pi_expect_reboot_p = 1;
        rpi_exit(0);
    }

    // whenever we push, yield to other thread.  slightly optimize
    // by waiting til its a multiple of 4.
    if(1 && chan_cnt(&net.unix_to_pi) % 4 == 0) {
        if(net.pi_exited_p)
            panic("BUG: deadlock: unix waiting for pi, which exited already\n");
        rpi_yield();
    }
}

void put_uint32(int fd, uint32_t u) { 
    net_debug(LOG_PUTGET, "unix put32: %x [%s]\n", u, boot_op_to_str(u));
    put_uint8(fd, (u>>0)&0xff);
    put_uint8(fd, (u>>8)&0xff);
    put_uint8(fd, (u>>16)&0xff);
    put_uint8(fd, (u>>24)&0xff);
}

uint8_t get_uint8(int fd) {
    if(!chan_cnt(&net.pi_to_unix)) {
        net_debug(LOG_FLOW, "unix no data: yielding\n");
        rpi_yield();
    }

    // if pi is dead this also works.
    if(!chan_cnt(&net.pi_to_unix))
        panic("should only get back here after pi side switched\n");

    uint8_t b = chan_pop(&net.pi_to_unix);
    net_debug(LOG_PUTGET, "unix returning %x\n", b);
    return b;
}

// we do 4 distinct get_uint8's b/c the bytes get dribbled back to 
// use one at a time --- when we try to read 4 bytes at once it can
// (occassionally!) fail.
//
// NOTE: a more general approach: we could make a version of 
// read_exact that will keep trying until a timeout.
//
// note: the other way to do is to assign these to a char array b and 
//      return *(unsigned)b
// however, the compiler doesn't have to align b to what unsigned 
// requires, so this can go awry.  easier to just do the simple way.
// 
// note: we use seperate statements (ending with ";") to force get_byte8(fd) 
// to get called in the right order.  the shorter, simpler:
//    (get_byte(fd) | get_byte(fd) << 8 ...)
// isn't guaranteed to be called in that order b/c '|' is not a seq point.
uint32_t get_uint32(int fd) {
    uint32_t u = get_uint8(fd);
    u |= get_uint8(fd) << 8;
    u |= get_uint8(fd) << 16;
    u |= get_uint8(fd) << 24;

    net_debug(LOG_PUTGET, "unix get32: ret=%x [%s]\n", u, boot_op_to_str(u));
    return u;
}

#include "put-code.c"

// what's the cleanest way to run 
void unix_server(void *ctx) {
    net_debug(LOG_FLOW, "in unix thread\n");

    // should return an error if failed, smh.
    simple_boot(0, (uint32_t)net.recv, net.send, net.nbytes);
    
    net_debug(LOG_EXIT, "SIM: UNIX success, sent code and got BOOT_SUCCESS\n");
    net.unix_exited_p = 1;
    net.unix_success_p = 1;
    rpi_exit(0);
}
