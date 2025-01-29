/* 
 * engler, cs140e: simple unix-side bootloader implementation.  
 * see the lab README.md for the protocol definition.
 *
 * all your modifications should go here.
 */
#include "put-code.h"
#include "boot-crc32.h"
#include "boot-defs.h"

/************************************************************************
 * helper code: you shouldn't have to modify this.
 */


#ifndef __RPI__
// write_exact will trap errors.
void put_uint8(int fd, uint8_t b)   { write_exact(fd, &b, 1); }
void put_uint32(int fd, uint32_t u) { write_exact(fd, &u, 4); }

uint8_t get_uint8(int fd) {
    uint8_t b;

    int res;
    if((res = read(fd, &b, 1)) < 0) 
        die("my-install: tty-USB read() returned error=%d: disconnected?\n", res);
    if(res == 0)
        die("my-install: tty-USB read() returned 0 bytes.  r/pi not responding [reboot it?]\n");

    // impossible for anything else.
    assert(res == 1);
    return b;
}

// we do 4 distinct get_uint8's b/c the bytes get dribbled back to 
// use one at a time --- when we try to read 4 bytes at once it will
// fail.
//
// note: the other way to do is to assign these to a char array b and 
//  return *(unsigned)b
// however, the compiler doesn't have to align b to what unsigned 
// requires, so this can go awry.  easier to just do the simple way.
// we do with |= to force get_byte to get called in the right order 
//  (get_byte(fd) | get_byte(fd) << 8 ...) 
// isn't guaranteed to be called in that order b/c '|' is not a seq point.
uint32_t get_uint32(int fd) {
    uint32_t u;
    u  = get_uint8(fd);
    u |= get_uint8(fd) << 8;
    u |= get_uint8(fd) << 16;
    u |= get_uint8(fd) << 24;
    return u;
}

#endif


#define boot_output(msg...) output("BOOT:" msg)

// helper tracing put/get routines: if you set 
//  <trace_p> = 1 
// you can see the stream of put/gets: makes it easy 
// to compare your bootloader to the ours and others.
//
// there are other ways to do this --- this is
// clumsy, but simple.
//
// NOTE: we can intercept these puts/gets transparently
// by interposing on a file, a socket or giving the my-install
// a file descriptor to use.

// set this to one to trace.
int trace_p = 0;

// recv 8-bits: if we are tracing, print.
static inline uint8_t trace_get8(int fd) {
    uint8_t v = get_uint8(fd);
    if(trace_p)
        trace("GET8:%x\n", v);
    return v;
}

// recv 32-bits: if we are tracing, print.
static inline uint32_t trace_get32(int fd) {
    uint32_t v = get_uint32(fd);
    if(trace_p)
        trace("GET32:%x [%s]\n", v, boot_op_to_str(v));
    return v;
}

// send 8 bits: if we are tracing print.
static inline void trace_put8(int fd, uint8_t v) {
    // we assume put8 is the only way to write data.
    if(trace_p == TRACE_ALL)
        trace("PUT8:%x\n", v);
    put_uint8(fd, v);
}

// send 32 bits: if we are tracing print.
static inline void trace_put32(int fd, uint32_t v) {
    if(trace_p)
        trace("PUT32:%x [%s]\n", v, boot_op_to_str(v));
    put_uint32(fd,v);
}

// always call this routine on the first 32-bit word in any message
// sent by the pi.
//
// hack to handle unsolicited <PRINT_STRING>: 
//  1. call <get_op> for the first uint32 in a message.  
//  2. the code checks if it received a <PRINT_STRING> and emits if so;
//  3. otherwise returns the 32-bit value.
//
// error:
//  - only call IFF the word could be an opcode (see <simple-boot.h>).
//  - do not call it on data since it could falsely match a data value as a 
//    <PRINT_STRING>.
static inline uint32_t get_op(int fd) {
    // we do not trace the output from PRINT_STRING so do not call the
    // tracing operations here except for the first word after we are 
    // sure it is not a <PRINT_STRING>
    uint32_t op = get_uint32(fd);
    if(op != PRINT_STRING) {
        if(trace_p)
            trace("GET32:%x [%s]\n", op, boot_op_to_str(op));
        return op;
    }

    // NOTE: we do not trace this code.
    debug_output("PRINT_STRING:");
    unsigned nbytes = get_uint32(fd);
    demand(nbytes < 512, pi sent a suspiciously long string);
    output("pi sent print: <");
    for(int i = 0; i < nbytes-1; i++)
        output("%c", get_uint8(fd));

    // eat the trailing newline to make it easier to compare output.
    uint8_t c = get_uint8(fd);
    if(c != '\n')
        output("%c", c);
    output(">\n");

    // attempt to get a non <PRINT_STRING> op.
    return get_op(fd);
}

// helper routine to make <simple_boot> code cleaner:
//   - check that the expected value <exp> is equal the the value we <got>.
//   - on mismatch, drains the tty and echos (to help debugging) and then 
//     dies.
static void 
boot_check(int fd, const char *msg, unsigned exp, unsigned got) {
    if(exp == got)
        return;

    // XXX: need to check: can there be garbage in the /tty when we
    // first open it?  If so, we should drain it.
    output("%s: expected %x [%s], got %x [%s]\n", msg, 
            exp, 
            boot_op_to_str(exp), 
            got,
            boot_op_to_str(got));

#ifndef __RPI__
    // after error: just echo the pi output so we can kind of see what is going
    // on.   <TRACE_FD> is used later.
    unsigned char b;
    while(fd != TRACE_FD && read(fd, &b, 1) == 1) {
        // fputc(b, stderr);
        fprintf(stderr, "%c [%d]", b,b);
    }
#endif
    panic("pi-boot failed\n");
}

//**********************************************************************
// The unix side bootloader code: you implement this.
// 
// Implement steps
//    1,2,3,4.
//
//  0 and 5 are implemented as demonstration.
//
// Note: if timeout in <set_tty_to_8n1> is too small (set by our caller)
// you can fail here. when you do a read and the pi doesn't send data 
// quickly enough.
//
// <boot_addr> is sent with <PUT_PROG_INFO> as the address to run the
// code at.
void simple_boot(int fd, uint32_t boot_addr, const uint8_t *buf, unsigned n) { 
    // all implementations should have the same message: same bytes,
    // same crc32: cross-check these values to detect if your <read_file> 
    // is busted.
    trace("simple_boot: sending %d bytes, crc32=%x\n", n, crc32(buf,n));
    boot_output("waiting for a start\n");

    // NOTE: only call <get_op> to assign to the <op> var.
    uint32_t op;

    // step 0: drain the initial data.  can have garbage.  
    // 
    // the code is a bit odd b/c 
    // if we have a single garbage byte, then reading 32-bits will
    // will not match <GET_PROG_INFO> (obviously) and if we keep reading 
    // 32 bits then we will never sync up with the input stream, our hack
    // is to read a byte (to try to sync up) and then go back to reading 32-bit
    // ops.
    //
    // CRUCIAL: make sure you use <get_op> for the first word in each 
    // message.
    while((op = get_op(fd)) != GET_PROG_INFO) {
        output("expected initial GET_PROG_INFO, got <%x>: discarding.\n", op);
        // have to remove just one byte since if not aligned, stays not aligned
        get_uint8(fd);
    } 

    // 1. reply to the GET_PROG_INFO
    todo("reply to GET_PROG_INFO");

    // 2. drain any extra GET_PROG_INFOS
    todo("drain any extra GET_PROG_INFOS");

    // 3. check that we received a GET_CODE
    todo("check that we received a GET_CODE");

    // 4. handle it: send a PUT_CODE + the code.
    todo("send PUT_CODE + the code in <buf>");

    // 5. Wait for BOOT_SUCCESS
    todo("wait for BOOT_SUCCESS");

    boot_output("bootloader: Done.\n");
}
