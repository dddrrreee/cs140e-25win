/*
 * simple example of how to do a system call.
 * you have to finish:
 *   1. <syscall_vector> (below) to get the syscall num 
 *   2. <software_interrupt_asm> in <interrupts-asm.S>
 *     to call it correctly.  NOTE: for this step we run at the 
 *     same level so don't update the stack pointer sp.
 */
#include "rpi.h"
#include "rpi-interrupts.h"

int syscall_hello(const char *msg);
int syscall_illegal(void);

// client has to define this.
void interrupt_vector(unsigned pc) {
    panic("should not be called\n");
}

// pc should point to the system call instruction.
//      can see the encoding on a3-29:  lower 24 bits hold the encoding.
// r0 = the first argument passed to the system call.
int syscall_vector(unsigned pc, uint32_t r0) {
    uint32_t inst=0, sys_num=0;

    // figure out the instruction and the system call number.
    unimplemented();
    trace("inst=%b, sys_num=%d\n", inst, sys_num);

    switch(sys_num) {
    case 1: 
            trace("syscall: <%s>\n", (const char *)r0); 
            return 0;
    default: 
            trace("illegal system call = %d!\n", sys_num);
            return -1;
    }
}


void notmain() {
    unsigned sp;
    asm volatile ("mov %0, sp" : "=r"(sp)); 
    printk("current stackptr = %x\n", sp);

    trace("about to install handlers\n");
    interrupt_init();

    trace("about to run syscall hello\n");
    int res = syscall_hello("hello world");
    trace("result of calling system call 1=%d\n", res);
    if(res != 0)
        panic("expected result=0, have: %d\n", res);

    trace("about to call illegal system call: should die\n");
    res = syscall_illegal();
    trace("result of illegal system call =%d\n", res);
    if(res != -1)
        panic("expected result=-1, have: %d\n", res);
    trace("SUCCESS!\n");
}
