/* 
    save all registers in ascending order.

        TRACE:do_syscall:reg[1]=0x1
        TRACE:do_syscall:reg[2]=0x2
        TRACE:do_syscall:reg[3]=0x3
        TRACE:do_syscall:reg[4]=0x4
        TRACE:do_syscall:reg[5]=0x5
        TRACE:do_syscall:reg[6]=0x6
        TRACE:do_syscall:reg[7]=0x7
        TRACE:do_syscall:reg[8]=0x8
        TRACE:do_syscall:reg[9]=0x9
        TRACE:do_syscall:reg[10]=0xa
        TRACE:do_syscall:reg[11]=0xb
        TRACE:do_syscall:reg[12]=0xc
        TRACE:do_syscall:reg[13]=0xd
        TRACE:do_syscall:reg[14]=0xe
        TRACE:do_syscall:reg[15]=0x80e4
        TRACE:do_syscall:reg[16]=0x10

 */
#include "rpi.h"
#include "asm-helpers.h"
#include "cpsr-util.h"
#include "vector-base.h"

int do_syscall(uint32_t regs[17]) {
    int sysno = regs[0];
    trace("in syscall: sysno=%d\n", sysno);

    for(unsigned i = 0; i < 17; i++)
        if(regs[i])
            trace("reg(%d)=%x\n", i, regs[i]);

    assert(sysno == 0);
    clean_reboot();
}

void swi_fn(void);
void switchto_user_asm(uint32_t regs[17]);

void nop_10(void);
void mov_ident(void);

void notmain(void) {
    extern uint32_t swi_test_handlers[];
    vector_base_set(swi_test_handlers);

    output("about to check that swi test works\n");

    uint32_t regs[17];
    // similar to <staff-start.S:mov_ident> set each register 
    // to its value (r1 = 1, r2 = 2, etc).   we should get
    // the same values printed as in <2-reg-save.c>
    for(unsigned i = 0; i < 15; i++)
        regs[i] = i;
    // addresss of swi instruction in <staff-start.S:mov_ident>
    regs[15] = (uint32_t)swi_fn;   
    regs[16] = USER_MODE;
    trace("about to jump to pc=[%x] with cpsr=%x\n",
            regs[15], regs[16]);
    switchto_user_asm(regs);
}
